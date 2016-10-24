/*  Impulse Response Processor XMMS plugin
 *  Low Latency Version
 *
 *  Copyright (C) 2006-2010 Teru KAMOGASHIRA
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "fv3_config.h"
#include <typeinfo>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <libsamplerate2/samplerate2.h>
#include <sndfile.h>
#include <freeverb/irmodel.hpp>
#include <freeverb/irmodel2.hpp>
#include <freeverb/irmodel2zl.hpp>
#include <freeverb/irmodel3.hpp>
#include <freeverb/utils.hpp>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>
#include <gdither.h>
#include "CFileLoader.hpp"
#include "xmms_defs.h"

#ifdef PLUGDOUBLE
typedef fv3::irbase_ IRBASE;
typedef fv3::irmodel_ IRMODEL;
typedef fv3::irmodel2_ IRMODEL2;
typedef fv3::irmodel2zl_ IRMODEL2ZL;
typedef fv3::irmodel3_ IRMODEL3;
typedef fv3::utils_ UTILS;
typedef double pfloat_t;
typedef fv3::CFileLoader_ CFILELOADER;
typedef fv3::slot_ SLOTP;
#else
typedef fv3::irbase_f IRBASE;
typedef fv3::irmodel_f IRMODEL;
typedef fv3::irmodel2_f IRMODEL2;
typedef fv3::irmodel2zl_f IRMODEL2ZL;
typedef fv3::irmodel3_f IRMODEL3;
typedef fv3::utils_f UTILS;
typedef float pfloat_t;
typedef fv3::CFileLoader_f CFILELOADER;
typedef fv3::slot_f SLOTP;
#endif

class ReverbVector
{
public:
  ReverbVector(){ rv.clear(); }
  ~ReverbVector(){ clear(); }
  IRBASE * at(unsigned int i){ return rv[i]; }
  IRBASE * operator[](unsigned int i){ return rv[i]; }
  IRBASE * assign(unsigned int i, const char * type)
  {
    delete rv.at(i);
    rv[i] = new_model(type);
    return rv[i];
  }
  IRBASE * push_back(const char * type)
  {
    IRBASE * model = new_model(type);
    rv.push_back(model);
    return model;
  }
  void pop_back()
  {
    delete rv[size()-1];
    rv.pop_back();
  }
  void clear()
  {
    for(std::vector<IRBASE*>::iterator i = rv.begin();
	i != rv.end();i ++) delete *i;
    rv.clear();
  }
  unsigned int size(){ return rv.size(); }
private:
  std::vector<IRBASE*> rv;
  IRBASE * new_model(const char * type)
  {
    IRBASE * model = NULL;
    if(strcmp(type, "irmodel") == 0)
      model = new IRMODEL;
    if(strcmp(type, "irmodel2") == 0)
      model = new IRMODEL2;
    if(strcmp(type, "irmodel2zl") == 0)
      model = new IRMODEL2ZL;
    if(strcmp(type, "irmodel3") == 0)
      model = new IRMODEL3;
    if(model == NULL)
      model = new IRMODEL2;
    return model;
  }
};

static const char *about_text = 
  "Freeverb3 "VERSION"\n"
  "Impulse Response Processor (v3F)\n"
  "XMMS/BMP/Audacious Plugin\n"
#ifdef PLUGDOUBLE
  "Double Precision Version\n"
#else
  "Single Precision Version\n"
#endif
  "SIMD: "
#ifdef ENABLE_3DNOW
  "3DNow! "
#endif
#ifdef ENABLE_SSE
  "SSE "
#endif
#ifdef ENABLE_SSE_V2
  "SSE_V2 "
#endif
#ifdef ENABLE_SSE2
  "SSE2 "
#endif
#ifdef ENABLE_SSE3
  "SSE3 "
#endif
#ifdef ENABLE_SSE4
  "SSE4 "
#endif
  "\n"
  "Copyright (C) 2006-2010 Teru KAMOGASHIRA\n"
  "http://freeverb3.sourceforge.net/";

static const char *productString = "Freeverb3 "VERSION" [Impulser2(v3F)]";
static const char *configSectionString = "freeverb3_plugin_irmodel2";

static bool validModel = false;
static int StreamFs = 0;
// Latency
static int latencyIndex = 0, conf_latency_index = 4;
static const int presetLatencyMax = 7;
static const char presetLatencyString[][32] =
  {"1024|512x8", "2048|512x16", "4096|512x32",
   "8192|1024x8", "16384|1024x16(Default)", "32768|1024x32", "65536|2048x8",};
static const long presetLatencyValue[] =
  {1024, 2048, 4096, 8192, 16384, 32768, 65536,};
static const long presetLatencyValue1[] =
  {512, 512, 512, 1024, 1024, 1024, 2048,};
static const long presetLatencyValue2[] =
  {8,   16,  32,  8,    16,   32,   8,};

// mono/stereo/swap slot
static const int presetSlotModeMax = 3;
static const char presetSlotModeString[][32] =
  {"Mono((L+R)/2) =>> Stereo", "Stereo =>> Stereo", "Swap LR",};
static const int presetSlotModeValue[] =
  {1, 0, 3,};

// dithering
static const int presetDitherMax = 4;
static const char presetDitherString[][16] =
  {"GDitherNone", "GDitherRect", "GDitherTri", "GDitherShaped"};
static const GDitherType presetDitherValue[] =
  {GDitherNone, GDitherRect, GDitherTri, GDitherShaped,};
static int conf_dithering = -1, next_dithering = 3;
static GDither pdither;
static gboolean gdither_on = FALSE;

static const int presetIRModelMax = 4;
static const char presetIRModelString[][32] =
  { "fv3::irmodel2", "fv3::irmodel3 (Zero Latency)",
    "fv3::irmodel2zl (Zero Latency)", "fv3::irmodel",};
static const char presetIRModelValue[][16] =
  {"irmodel2", "irmodel3", "irmodel2zl", "irmodel",};
static int conf_rev_zl = 1;

// SLOT1 DRY + WET
// SLOT2~ WET only

typedef struct{
  float wet, dry, lpf, hpf, width, stretch, limit, idelay;
  int i1o2_index, valid;
  std::string filename, inf;
} SlotConfiguration;

#define SLOT_MAX 32
static int slotNumber = 1, currentSlot = 1;

// These should be initialized in init() and
// cleaned in cleanup()
static std::vector<SlotConfiguration> * slotVector = NULL;
// ... and only mod_samples() operate these vectors
static std::vector<SlotConfiguration> * currentSlotVector = NULL;
static ReverbVector * reverbVector = NULL;
static pthread_mutex_t plugin_mutex;
static gboolean plugin_available = false;
#define MAX_KEY_STR_LENGTH 1024
static char key_i_string[MAX_KEY_STR_LENGTH];
static const char * key_i(const char * str, int i)
{
  if(i == 1) return str;
  else
    {
      sprintf(key_i_string, "%s__%d", str, i);
      return key_i_string;
    }
}

static void slot_init(SlotConfiguration * slot)
{
#ifdef DEBUG
  fprintf(stderr, "Impulser2: slot_init\n");
#endif
  slot->wet = -28.0f;
  slot->lpf = 0.0f;
  slot->hpf = 0.0f;
  slot->width = 1.0f;
  slot->stretch = 0.0f;
  slot->limit = 100.0f;
  slot->idelay = 0.0f;
  slot->i1o2_index = 1;
  slot->valid = 0;
}

static void slot_save(SlotConfiguration * slot, int i)
{
#ifdef DEBUG
  fprintf(stderr, "Impulser2: slot_save %d\n", i);
#endif
#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("wet",i)),slot->wet);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("dry",i)),slot->dry);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("width",i)),slot->width);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("LPF",i)),slot->lpf);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("HPF",i)),slot->hpf);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("stretch",i)),slot->stretch);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("limit",i)),slot->limit);
  xmms_cfg_write_float(cfg, const_cast<char*>(configSectionString),const_cast<char*>(key_i("idelay",i)),slot->idelay);
  xmms_cfg_write_int(cfg,const_cast<char*>(configSectionString),const_cast<char*>(key_i("i1o2_index",i)),slot->i1o2_index);
  xmms_cfg_write_string(cfg,const_cast<char*>(configSectionString),const_cast<char*>(key_i("file",i)),(gchar*)slot->filename.c_str());
  xmms_cfg_write_default_file(cfg);
  xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("wet",i), slot->wet);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("dry",i), slot->dry);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("width",i), slot->width);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("LPF",i), slot->lpf);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("HPF",i), slot->hpf);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("stretch",i), slot->stretch);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("limit",i), slot->limit);
  bmp_cfg_db_set_float (cfg, configSectionString, key_i("idelay",i), slot->idelay);
  bmp_cfg_db_set_int   (cfg, configSectionString, key_i("i1o2_index",i), slot->i1o2_index);
  bmp_cfg_db_set_string(cfg, configSectionString, key_i("file",i), (gchar*)slot->filename.c_str());
  bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
#endif
  aud_cfg_db_set_float (cfg, configSectionString, key_i("wet",i), slot->wet);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("dry",i), slot->dry);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("width",i), slot->width);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("LPF",i), slot->lpf);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("HPF",i), slot->hpf);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("stretch",i), slot->stretch);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("limit",i), slot->limit);
  aud_cfg_db_set_float (cfg, configSectionString, key_i("idelay",i), slot->idelay);
  aud_cfg_db_set_int   (cfg, configSectionString, key_i("i1o2_index",i), slot->i1o2_index);
  aud_cfg_db_set_string(cfg, configSectionString, key_i("file",i), (gchar*)slot->filename.c_str());
  aud_cfg_db_close(cfg);
#endif
#endif
}

static void slot_load(SlotConfiguration * slot, int i)
{
#ifdef DEBUG
  fprintf(stderr, "Impulser2: slot_load %d\n", i);
#endif
#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  gchar * filename;
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("wet",i)), &slot->wet);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("dry",i)), &slot->dry);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("width",i)), &slot->width);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("LPF",i)), &slot->lpf);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("HPF",i)), &slot->hpf);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("stretch",i)), &slot->stretch);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("limit",i)), &slot->limit);
  xmms_cfg_read_float (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("idelay",i)), &slot->idelay);
  xmms_cfg_read_int   (cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("i1o2_index",i)), &slot->i1o2_index);
  gboolean ok = xmms_cfg_read_string(cfg, const_cast<char*>(configSectionString), const_cast<char*>(key_i("file",i)), &filename);
  if(ok == TRUE) slot->filename = filename;
  else slot_init(slot);
  xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  gchar * filename;
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("wet",i), &slot->wet);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("dry",i), &slot->dry);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("width",i), &slot->width);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("LPF",i), &slot->lpf);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("HPF",i), &slot->hpf);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("stretch",i), &slot->stretch);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("limit",i), &slot->limit);
  bmp_cfg_db_get_float (cfg, configSectionString, key_i("idelay",i), &slot->idelay);
  bmp_cfg_db_get_int   (cfg, configSectionString, key_i("i1o2_index",i), &slot->i1o2_index);
  gboolean ok = bmp_cfg_db_get_string(cfg, configSectionString, key_i("file",i), &filename);
  if(ok == TRUE) slot->filename = filename;
  else slot_init(slot);
  bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
#endif
  gchar * filename;
  aud_cfg_db_get_float (cfg, configSectionString, key_i("wet",i), &slot->wet);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("dry",i), &slot->dry);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("width",i), &slot->width);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("LPF",i), &slot->lpf);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("HPF",i), &slot->hpf);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("stretch",i), &slot->stretch);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("limit",i), &slot->limit);
  aud_cfg_db_get_float (cfg, configSectionString, key_i("idelay",i), &slot->idelay);
  aud_cfg_db_get_int   (cfg, configSectionString, key_i("i1o2_index",i), &slot->i1o2_index);
  gboolean ok = aud_cfg_db_get_string(cfg, configSectionString, key_i("file",i), &filename);
  if(ok == TRUE) slot->filename = filename;
  else slot_init(slot);
  aud_cfg_db_close(cfg);
#endif
#endif
}

static int protectValue = 0;

static void set_rt_reverb(IRBASE * reverbm, SlotConfiguration * slot)
{
  reverbm->setwet(slot->wet);
  reverbm->setdry(0);
  reverbm->setLPF(slot->lpf);
  reverbm->setHPF(slot->hpf);
  reverbm->setwidth(slot->width);
  if(reverbVector->size() > 0&&slotVector->size() > 0)
    (*reverbVector)[0]->setdry((*slotVector)[0].dry);
}

// libsndfile

static char inf[1024] = "";

static int store_inf(const char * file)
{
  SF_INFO rsfinfo;
  SNDFILE * sndFile = sf_open(file, SFM_READ, &rsfinfo);
  if(sndFile == NULL)
    {
      fprintf(stderr, "Impulser2: store_inf: sf_open: Couldn't load \"%s\"\n", file);
      return -1;
    }
  float second = (float)rsfinfo.frames/(float)rsfinfo.samplerate;
  sprintf(inf, "%lld[samples] %d[Hz] %d[Ch] %f[s]",
	  (long long int)rsfinfo.frames, rsfinfo.samplerate, rsfinfo.channels, second);
  sf_close(sndFile);
  return 0;
}

// Logo
#include "wave.xpm"

// Splash
static GtkWidget *splashWindow = NULL, *logo = NULL;

static void hideSplash()
{
  if(splashWindow == NULL) return;
  gtk_widget_destroy(GTK_WIDGET(splashWindow));
  splashWindow = NULL;
}

static void showSplash(const char * c1, const char * c2, const char * c3)
{
  if (splashWindow != NULL) return;
  GtkWidget *splashTable = NULL, *splashLabel1 = NULL, *splashLabel2 = NULL, *splashLabel3 = NULL, *button = NULL;
  GtkStyle *style;
  GdkPixmap * logopix;
  GdkBitmap * logomask;

  splashWindow = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_widget_realize(splashWindow);
  style = gtk_widget_get_style(splashWindow);
  logopix = gdk_pixmap_create_from_xpm_d(splashWindow->window, &logomask,
					 &style->bg[GTK_STATE_NORMAL], (gchar **) wave_xpm);
  logo = gtk_pixmap_new(logopix,logomask);
  
  splashLabel1 = gtk_label_new(c1);
  splashLabel2 = gtk_label_new(c2);
  splashLabel3 = gtk_label_new(c3);
  splashTable = gtk_table_new(5, 5, FALSE);
  gtk_table_attach(GTK_TABLE(splashTable), logo,         0,5, 0,1, GTK_FILL, GTK_FILL, 0,0);  
  gtk_table_attach(GTK_TABLE(splashTable), splashLabel1, 1,4, 1,2, GTK_FILL, GTK_FILL, 0,0);
  gtk_table_attach(GTK_TABLE(splashTable), splashLabel2, 1,4, 2,3, GTK_FILL, GTK_FILL, 0,0);
  gtk_table_attach(GTK_TABLE(splashTable), splashLabel3, 1,4, 3,4, GTK_FILL, GTK_FILL, 0,0);
  
  button = gtk_button_new_with_label("Ok");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(hideSplash), NULL);
  gtk_table_attach(GTK_TABLE(splashTable), button, 2,3, 5,6, GTK_FILL, GTK_FILL, 0,0);

  gtk_container_add(GTK_CONTAINER(splashWindow), splashTable);
  gtk_container_set_border_width(GTK_CONTAINER(splashWindow), 10);
  gtk_window_set_position(GTK_WINDOW(splashWindow), GTK_WIN_POS_CENTER); 
  gtk_widget_show_all(splashWindow);
#ifdef AUDACIOUS
  gtk_window_present(GTK_WINDOW(splashWindow));
#endif
}

static void about(void)
{
  showSplash("", about_text, "");
}

// file selector

static GtkWidget *file_selector = NULL, *show_filename = NULL, *show_inf = NULL, *err_dialog = NULL;
static gchar *selected_filename = NULL;
static std::string ir_filename = "", current_ir = "";

static void error_d()
{
  if(err_dialog != NULL)
    {
      gtk_widget_destroy(err_dialog);
      gtk_widget_destroyed(err_dialog, &err_dialog);
    }
}

static void cancel_selector(GtkFileSelection *selector, gpointer user_data)
{
  if(file_selector != NULL)
    {
      gtk_widget_destroy(file_selector);
      gtk_widget_destroyed(file_selector, &file_selector);
    }
  error_d();
}

static void store_filename(GtkFileSelection *selector, gpointer user_data)
{
  selected_filename = (gchar*)gtk_file_selection_get_filename (GTK_FILE_SELECTION(file_selector));
  if(store_inf(selected_filename) != 0)
    {
      if (err_dialog == NULL)
        {
#if __AUDACIOUS_PLUGIN_API__ >= 16
	  audgui_simple_message(&err_dialog, GTK_MESSAGE_ERROR, (gchar*)"Impulser2 Error", (gchar*)"Could not load IR file.");
#else
          err_dialog = _XMMS_DIALOG((gchar*)"Error", (gchar*)"Could not load IR file.", (gchar*)"Ok",FALSE,NULL,NULL);
          gtk_signal_connect(GTK_OBJECT(err_dialog), "destroy", GTK_SIGNAL_FUNC(error_d), NULL);
#endif
        }
    }
  else
    {
      ir_filename = selected_filename;
      (*slotVector)[currentSlot-1].filename = ir_filename;
      (*slotVector)[currentSlot-1].inf = inf;
      gtk_label_set_text(GTK_LABEL(show_filename), ir_filename.c_str());
      gtk_label_set_text(GTK_LABEL(show_inf), inf);
      gtk_widget_destroy(GTK_WIDGET(file_selector));
      file_selector = NULL;
      error_d();
    }
}

static void select_file(void)
{
  if (file_selector != NULL) return;
  file_selector = gtk_file_selection_new("Please select a Impulse Response wav file.");
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->ok_button),
		     "clicked", GTK_SIGNAL_FUNC (store_filename), NULL);
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_selector)->cancel_button),
		     "clicked", GTK_SIGNAL_FUNC (cancel_selector), NULL);
  gtk_signal_connect(GTK_OBJECT(file_selector),
		     "destroy", GTK_SIGNAL_FUNC(cancel_selector), NULL);
  selected_filename = (gchar*)ir_filename.c_str();
  gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selector), selected_filename);
  gtk_widget_set_usize(GTK_WIDGET(file_selector), 800, 600);
  gtk_window_set_position(GTK_WINDOW(file_selector), GTK_WIN_POS_CENTER);
  gtk_widget_show(file_selector);
}

// GUI

static GtkWidget *conf_rev_dialog = NULL;
static GtkObject *conf_rev_wet_adj, *conf_rev_dry_adj, *conf_rev_lpf_adj, *conf_rev_hpf_adj,
  *conf_rev_width_adj, *conf_rev_stretch_adj, *conf_rev_limit_adj, *conf_rev_idelay_adj;
// slots
static GtkObject *slotAdjustment;
static GtkWidget *slotWidget, *slotLabel;
static GtkOptionMenu *optionMenu_MSOption;

static void slot_show(SlotConfiguration * slot)
{
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_wet_adj), slot->wet);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_lpf_adj), slot->lpf);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_hpf_adj), slot->hpf);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_width_adj), slot->width);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_stretch_adj), slot->stretch);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_limit_adj), slot->limit);
  gtk_adjustment_set_value(GTK_ADJUSTMENT(conf_rev_idelay_adj), slot->idelay);
  gtk_option_menu_set_history(GTK_OPTION_MENU(optionMenu_MSOption), slot->i1o2_index);
  gtk_label_set_text(GTK_LABEL(show_filename), slot->filename.c_str());
  gtk_label_set_text(GTK_LABEL(show_inf), slot->inf.c_str());
}

static void write_config(void)
{
#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  xmms_cfg_write_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("latency_index"), conf_latency_index);
  xmms_cfg_write_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("dithering_mode"), conf_dithering);
  xmms_cfg_write_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("zero_latency"), conf_rev_zl);
  xmms_cfg_write_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("slotNumber"), slotNumber);
  xmms_cfg_write_default_file(cfg);
  xmms_cfg_free(cfg);
  for(int i = 0;i < slotNumber;i ++) slot_save(&(*slotVector)[i], i+1);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  bmp_cfg_db_set_int(cfg, configSectionString, "latency_index", conf_latency_index);
  bmp_cfg_db_set_int(cfg, configSectionString, "dithering_mode", conf_dithering);
  bmp_cfg_db_set_int(cfg, configSectionString, "zero_latency", conf_rev_zl);
  bmp_cfg_db_set_int(cfg, configSectionString, "slotNumber", slotNumber);
  bmp_cfg_db_close(cfg);
  for(int i = 0;i < slotNumber;i ++) slot_save(&(*slotVector)[i], i+1);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
#endif
  aud_cfg_db_set_int(cfg, configSectionString, "latency_index", conf_latency_index);
  aud_cfg_db_set_int(cfg, configSectionString, "dithering_mode", conf_dithering);
  aud_cfg_db_set_int(cfg, configSectionString, "zero_latency", conf_rev_zl);
  aud_cfg_db_set_int(cfg, configSectionString, "slotNumber", slotNumber);
  aud_cfg_db_close(cfg);
  for(int i = 0;i < slotNumber;i ++) slot_save(&(*slotVector)[i], i+1);
#endif
#endif
}

static GtkWidget *applyButton = NULL;

// apply changes immediately if RT parameters were changed
static void conf_apply_realtime_sig(GtkButton * button, gpointer data)
{
  if(protectValue == 0)
    {
      (*slotVector)[currentSlot-1].wet = GTK_ADJUSTMENT(conf_rev_wet_adj)->value;
      (*slotVector)[currentSlot-1].lpf = GTK_ADJUSTMENT(conf_rev_lpf_adj)->value;
      (*slotVector)[currentSlot-1].hpf = GTK_ADJUSTMENT(conf_rev_hpf_adj)->value;
      (*slotVector)[currentSlot-1].width = GTK_ADJUSTMENT(conf_rev_width_adj)->value;
      (*slotVector)[0].dry = GTK_ADJUSTMENT(conf_rev_dry_adj)->value;
      if(currentSlot <= (int)reverbVector->size())
	set_rt_reverb((*reverbVector)[currentSlot-1], &(*slotVector)[currentSlot-1]);
    }
}

// enable apply button if non-RT parameters were changed
static void conf_non_realtime_sig(GtkButton * button, gpointer data)
{
  if(protectValue == 0)
    {
      if(applyButton != NULL) gtk_widget_set_sensitive(applyButton, TRUE);
    }
}

// Apply non-RT parameters
static void conf_rev_apply_cb(GtkButton * button, gpointer data)
{
  if(currentSlot <= (int)slotVector->size())
    {
      (*slotVector)[currentSlot-1].stretch = GTK_ADJUSTMENT(conf_rev_stretch_adj)->value;
      (*slotVector)[currentSlot-1].limit = GTK_ADJUSTMENT(conf_rev_limit_adj)->value;
      (*slotVector)[currentSlot-1].idelay = GTK_ADJUSTMENT(conf_rev_idelay_adj)->value;
      if(applyButton != NULL) gtk_widget_set_sensitive(applyButton, FALSE);
    }
}

static void conf_rev_default_cb(GtkButton * button, gpointer data)
{
  protectValue = 1;
  slot_init(&(*slotVector)[currentSlot-1]);
  slot_show(&(*slotVector)[currentSlot-1]);
  set_rt_reverb((*reverbVector)[currentSlot-1], &(*slotVector)[currentSlot-1]);
  protectValue = 0;
  if(applyButton != NULL) gtk_widget_set_sensitive(applyButton, FALSE);
}

static void conf_rev_select_cb(GtkButton * button, gpointer data)
{
  select_file();
}

static void conf_rev_ok_cb(GtkButton * button, gpointer data)
{
  conf_rev_apply_cb(button, data);
  write_config();
}

static void conf_rev_cancel_cb(GtkButton * button, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(conf_rev_dialog));
}

static void conf_set_i1o2(GtkButton * button, gpointer data)
{
  if(protectValue == 0)
    {
      fprintf(stderr, "Impulser2: I1O2 %s(%d)\n", presetSlotModeString[GPOINTER_TO_INT(data)], GPOINTER_TO_INT(data));
      (*slotVector)[currentSlot-1].i1o2_index = GPOINTER_TO_INT(data);
    }
}

static void conf_set_dithering(GtkToggleButton * button, gpointer data)
{
  fprintf(stderr, "Impulser2: set_dithering(%d)=%s\n",
	  GPOINTER_TO_INT(data), presetDitherString[GPOINTER_TO_INT(data)]);
  next_dithering = GPOINTER_TO_INT(data);
}

static void conf_set_latency(GtkButton * button, gpointer data)
{
  fprintf(stderr, "Impulser2: set_latency(%d)=%s\n",
	  GPOINTER_TO_INT(data), presetLatencyString[GPOINTER_TO_INT(data)]);
  conf_latency_index = GPOINTER_TO_INT(data);
}

static void conf_set_irmodel(GtkToggleButton * button, gpointer data)
{
  fprintf(stderr, "Impulser2: set_irmodel(%d)[%s]<%s>\n",
	  GPOINTER_TO_INT(data), presetIRModelString[GPOINTER_TO_INT(data)], presetIRModelValue[GPOINTER_TO_INT(data)]);
  if(conf_rev_zl != GPOINTER_TO_INT(data))
    {
      conf_rev_zl = GPOINTER_TO_INT(data);
      StreamFs = 0; // reset all slot
    }
}

static void conf_slot_inc_sig(GtkButton * button, gpointer data)
{
  fprintf(stderr, "Impulser2: WARNING: increasing slot during play is not supported!!\n");
  if(slotNumber >= SLOT_MAX) return;

  pthread_mutex_lock(&plugin_mutex);

  std::ostringstream os;
  os << slotNumber+1;
  gtk_label_set_text(GTK_LABEL(slotLabel), os.str().c_str());
  gint current = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(slotWidget));
  slotAdjustment = gtk_adjustment_new(current, 1, slotNumber+1, 1, 1, 0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(slotWidget), GTK_ADJUSTMENT(slotAdjustment),1,0);
  
  SlotConfiguration slotC;
  slot_load(&slotC, slotNumber+1);
  if(store_inf(slotC.filename.c_str()) != 0) sprintf(inf, "(not loaded)");
  slotC.inf = inf;
  slotVector->push_back(slotC);
  
  slotNumber ++;
  fprintf(stderr, "Impulser2: slot_inc: (*slotVector)[%d]\n", (int)slotVector->size());

  pthread_mutex_unlock(&plugin_mutex);
}

static void conf_slot_dec_sig(GtkButton * button, gpointer data)
{
  fprintf(stderr, "Impulser2: WARNING: decreasing slot during play is not supported!!\n");
  if(slotNumber <= 1) return;

  pthread_mutex_lock(&plugin_mutex);

  std::ostringstream os;
  os << slotNumber-1;
  gtk_label_set_text(GTK_LABEL(slotLabel), os.str().c_str());
  
  slot_save(&(*slotVector)[slotVector->size()-1], slotVector->size());
  slotVector->pop_back();
  
  gint current = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(slotWidget));
  if(current > slotNumber-1) current = slotNumber-1;
  slotAdjustment = gtk_adjustment_new(current, 1, slotNumber-1, 1, 1, 0);
  gtk_spin_button_configure(GTK_SPIN_BUTTON(slotWidget), GTK_ADJUSTMENT(slotAdjustment),1,0);
  
  slotNumber --;
  fprintf(stderr, "Impulser2: slot_dec: (*slotVector)[%d]\n", (int)slotVector->size());

  pthread_mutex_unlock(&plugin_mutex);
}

static void conf_rev_slot_select_changed_sig(GtkSpinButton * button, gpointer data)
{
  conf_rev_apply_cb(NULL, NULL);
  gint value = gtk_spin_button_get_value_as_int(button);
#ifdef DEBUG
  fprintf(stderr, "Impulser2: UI slot %d -> %d/%ld\n", currentSlot, value, slotVector->size());
#endif
  currentSlot = value;
  protectValue = 1;
  if(currentSlot < 0) return;
  slot_show(&(*slotVector)[currentSlot-1]);
  protectValue = 0;
}

#define MIN_DB (-100.0)
#define MAX_DB (20.0)

static void configure(void)
{
  GtkWidget *button, *table, *label, *hscale, *bbox;
  if(conf_rev_dialog != NULL) return;
  
  if(validModel != true) return;
  
  conf_rev_dialog = gtk_dialog_new();
  gtk_signal_connect(GTK_OBJECT(conf_rev_dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &conf_rev_dialog);
  gtk_window_set_title(GTK_WINDOW(conf_rev_dialog), productString);

  conf_rev_wet_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].wet,      MIN_DB, MAX_DB+1.0, 0.01, 0.1, 1.0);
  conf_rev_dry_adj = gtk_adjustment_new((*slotVector)[0].dry,                  MIN_DB, MAX_DB+1.0, 0.01, 0.1, 1.0);
  conf_rev_lpf_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].lpf,         0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_rev_hpf_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].hpf,         0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_rev_width_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].width,     0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_rev_stretch_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].stretch,-6.0, 6.0+1.0, 0.01, 0.1, 1.0);
  conf_rev_limit_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].limit,     0.0, 100.0+1.0, 0.01, 0.1, 1.0);
  conf_rev_idelay_adj = gtk_adjustment_new((*slotVector)[currentSlot-1].idelay,-800.0, 800.0+1.0, 0.01, 0.1, 1.0);
  
#define LABELS 14 // size of labels
#define SHOWLABEL 6 // size of label which do not have gtk_adjustment
  const char * labels[64] = {"Wet [dB]", "*Dry [dB]", "1-Pole LPF", "1-Zero HPF",
			     "width", "Stretch [sqrt(2)^]", "IR Limit [%]", "Initial Delay [ms]",
			     "Impulse File:", "PCM information", "Mono/Stereo Slot",
			     "*Fragment Size (<irmodel2|>irmodel3)", "*IR Model Type", "*Dithering Mode",};
  
  table = gtk_table_new(LABELS, 5, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table), 1);
  gtk_container_set_border_width(GTK_CONTAINER(table), 1);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(conf_rev_dialog)->vbox), table, TRUE, TRUE, 1);
  // label
  for(int i = 0;i < LABELS;i ++)
    {
#ifdef DEBUG
      fprintf(stderr, "[%d]%s\n", i, labels[i]);
#endif
      label = gtk_label_new(labels[i]);
      gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i+1, GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show(label);
    }
  
  // adjustments
  GtkObject * objects[] =
    {conf_rev_wet_adj, conf_rev_dry_adj, conf_rev_lpf_adj, conf_rev_hpf_adj,
     conf_rev_width_adj, conf_rev_stretch_adj, conf_rev_limit_adj, conf_rev_idelay_adj};
  for(int i = 0;i < LABELS-SHOWLABEL;i ++)
    {
#ifdef DEBUG
      fprintf(stderr, "[%d]\n", i);
#endif
      hscale = gtk_hscale_new(GTK_ADJUSTMENT(objects[i]));
      gtk_widget_set_usize(hscale, 300, 35);
      gtk_scale_set_digits(GTK_SCALE(hscale), 2); // under .
      gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 5, i, i+1);
      gtk_widget_show(hscale);
    }
  // signal
  int isRealTimeObject[] =
    {1, 1, 1, 1, 1, 0, 0, 0,};
  for(int i = 0;i < LABELS-SHOWLABEL;i ++)
    {
      if(isRealTimeObject[i] == 1)
        gtk_signal_connect(GTK_OBJECT(objects[i]), "value_changed", GTK_SIGNAL_FUNC(conf_apply_realtime_sig), NULL);
      else
        gtk_signal_connect(GTK_OBJECT(objects[i]), "value_changed", GTK_SIGNAL_FUNC(conf_non_realtime_sig), NULL);
    }
  
  // filename + information
  show_filename = gtk_label_new((*slotVector)[currentSlot-1].filename.c_str());
  gtk_table_attach_defaults(GTK_TABLE(table), show_filename, 1, 5, LABELS-SHOWLABEL, LABELS-SHOWLABEL+1);
  gtk_widget_show(show_filename);
  show_inf = gtk_label_new((*slotVector)[currentSlot-1].inf.c_str());
  gtk_table_attach_defaults(GTK_TABLE(table), show_inf, 1, 5, LABELS-SHOWLABEL+1, LABELS-SHOWLABEL+2);
  gtk_widget_show(show_inf);

  // Mono/Stereo Slot
  GtkWidget *menu2 = gtk_menu_new();
  optionMenu_MSOption = (GtkOptionMenu*)gtk_option_menu_new();
  for(int i = 0;i < presetSlotModeMax;i++)
    {
      GtkWidget * item = gtk_menu_item_new_with_label(presetSlotModeString[i]);
      gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(conf_set_i1o2), GINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu2), item);
    }
  gtk_option_menu_remove_menu(optionMenu_MSOption);
  gtk_option_menu_set_menu(optionMenu_MSOption, menu2);
  gtk_table_attach(GTK_TABLE(table), (GtkWidget*)optionMenu_MSOption, 1, 2,
		   LABELS-SHOWLABEL+2, LABELS-SHOWLABEL+3, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_option_menu_set_history(optionMenu_MSOption, (*slotVector)[currentSlot-1].i1o2_index);
  gtk_widget_show(GTK_WIDGET(optionMenu_MSOption));
  
  // Latency/FragmentSize
  GtkWidget *menu = gtk_menu_new();
  GtkOptionMenu *optionMenu_LatencyOption = (GtkOptionMenu*)gtk_option_menu_new();
  for(int i = 0;i < presetLatencyMax;i++)
    {
      GtkWidget * item = gtk_menu_item_new_with_label(presetLatencyString[i]);
      gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(conf_set_latency), GINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu), item);
    }
  gtk_option_menu_remove_menu(optionMenu_LatencyOption);
  gtk_option_menu_set_menu(optionMenu_LatencyOption, menu);
  gtk_table_attach(GTK_TABLE(table), (GtkWidget*)optionMenu_LatencyOption, 1, 2,
		   LABELS-SHOWLABEL+3, LABELS-SHOWLABEL+4, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_option_menu_set_history(optionMenu_LatencyOption, conf_latency_index);
  gtk_widget_show(GTK_WIDGET(optionMenu_LatencyOption));
  
  // ZL mode
  GtkWidget *menu_IRModel = gtk_menu_new();
  GtkOptionMenu *optionMenu_IRModel = (GtkOptionMenu*)gtk_option_menu_new();
  for(int i = 0;i < presetIRModelMax;i++)
    {
      GtkWidget * item = gtk_menu_item_new_with_label(presetIRModelString[i]);
      gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(conf_set_irmodel), GINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu_IRModel), item);
    }
  gtk_option_menu_remove_menu(optionMenu_IRModel);
  gtk_option_menu_set_menu(optionMenu_IRModel, menu_IRModel);
  gtk_table_attach(GTK_TABLE(table), (GtkWidget*)optionMenu_IRModel, 1, 2,
		   LABELS-SHOWLABEL+4, LABELS-SHOWLABEL+5, GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_option_menu_set_history(optionMenu_IRModel, conf_rev_zl);
  gtk_widget_show(GTK_WIDGET(optionMenu_IRModel));

  // dithering
  GtkWidget * menu3 = gtk_menu_new();
  GtkOptionMenu * omenu3 = (GtkOptionMenu*)gtk_option_menu_new();
  for(int i = 0;i < presetDitherMax; i++)
    {
      GtkWidget * item = gtk_menu_item_new_with_label(presetDitherString[i]);
      gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(conf_set_dithering), GINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu3), item);
    }
  gtk_option_menu_remove_menu(omenu3);
  gtk_option_menu_set_menu(omenu3, menu3);
  gtk_table_attach(GTK_TABLE(table), (GtkWidget*)omenu3, 1, 2, LABELS-SHOWLABEL+5, LABELS-SHOWLABEL+6,
		   GTK_SHRINK, GTK_SHRINK, 0, 0);
  gtk_option_menu_set_history(omenu3, next_dithering);
  gtk_widget_show(GTK_WIDGET(omenu3));
  
  //
  gtk_widget_show(table);

  // ====
  button = gtk_hseparator_new();
  gtk_table_attach_defaults(GTK_TABLE(table), button, 0, 5, LABELS-SHOWLABEL+6, LABELS-SHOWLABEL+7);
  gtk_widget_show(GTK_WIDGET(button));
  gtk_widget_show(button);
  // ====
  
  // slots box
  bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 1);
  
  // buttons
  label = gtk_label_new("Slot:");
  gtk_box_pack_start(GTK_BOX(bbox), label, FALSE, FALSE, 0);
  gtk_widget_show(label);

  // slot selector
  slotAdjustment = gtk_adjustment_new(currentSlot, 1, slotNumber, 1, 1, 0);
  slotWidget = gtk_spin_button_new(GTK_ADJUSTMENT(slotAdjustment),1,0);
  gtk_box_pack_start(GTK_BOX(bbox), slotWidget, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(slotWidget), "changed", GTK_SIGNAL_FUNC(conf_rev_slot_select_changed_sig), NULL);
  gtk_widget_show(slotWidget);

  // slot numbers
  std::ostringstream os;
  os << slotNumber;
  slotLabel = gtk_label_new(os.str().c_str());
  gtk_box_pack_start(GTK_BOX(bbox), slotLabel, FALSE, FALSE, 0);
  gtk_widget_show(slotLabel);

  // slot +
  button = gtk_button_new_with_label("Slot +");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_slot_inc_sig), NULL);
  gtk_widget_show(button);

  // slot -
  button = gtk_button_new_with_label("Slot -");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_slot_dec_sig), NULL);
  gtk_widget_show(button);

  gtk_table_attach_defaults(GTK_TABLE(table), bbox, 0, 5, LABELS-SHOWLABEL+7, LABELS-SHOWLABEL+8);
  gtk_widget_show(GTK_WIDGET(bbox));
  gtk_widget_show(bbox);
  
  // ====

  // Controls
  bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 1);
  gtk_box_pack_start(GTK_BOX((GTK_DIALOG(conf_rev_dialog)->action_area)), bbox, FALSE, FALSE, 0);

  button = gtk_button_new_with_label("Save");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_rev_ok_cb), NULL);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);

  button = gtk_button_new_with_label("Close");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_rev_cancel_cb), NULL);
  gtk_widget_show(button);
  
  applyButton = button = gtk_button_new_with_label("Apply");
  gtk_widget_set_sensitive(applyButton, FALSE);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_rev_apply_cb), NULL);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label("Default");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_rev_default_cb), NULL);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label("Load IR");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_rev_select_cb), NULL);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label("About");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(about), NULL);
  gtk_widget_show(button);
  
  gtk_widget_show(bbox);

  gtk_window_set_position(GTK_WINDOW(conf_rev_dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show(conf_rev_dialog);
}

// plugin functions
static void
#ifdef __GNUC__
__attribute__ ((constructor))
#endif
plugin_init(void)
{
  std::fprintf(stderr, "Impulser2: plugin_init()\n");
  pthread_mutex_init(&plugin_mutex, NULL);
}

static void
#ifdef __GNUC__
__attribute__ ((destructor))
#endif
plugin_fini(void)
{
  std::fprintf(stderr, "Impulser2: plugin_fini()\n");
  pthread_mutex_destroy(&plugin_mutex);
}

static void init(void)
{
#ifndef __GNUC__
  plugin_init();
#endif
  fprintf(stderr, "Impulser2: init()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = true;

  if(validModel == false)
    {
      slotVector = new std::vector<SlotConfiguration>;
      currentSlotVector = new std::vector<SlotConfiguration>;
      reverbVector = new ReverbVector;
      validModel = true;
    }

#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  xmms_cfg_read_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("dithering_mode"), &conf_dithering);
  xmms_cfg_read_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("latency_index"), &conf_latency_index);
  xmms_cfg_read_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("zero_latency"), &conf_rev_zl);
  gboolean ok = xmms_cfg_read_int(cfg, const_cast<char*>(configSectionString), const_cast<char*>("slotNumber"), &slotNumber);
  xmms_cfg_free(cfg);
  if(ok == FALSE) slotNumber = 1;
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  bmp_cfg_db_get_int(cfg, configSectionString, "dithering_mode", &conf_dithering);
  bmp_cfg_db_get_int(cfg, configSectionString, "latency_index", &conf_latency_index);
  bmp_cfg_db_get_int(cfg, configSectionString, "zero_latency", &conf_rev_zl);
  gboolean ok = bmp_cfg_db_get_int(cfg, configSectionString, "slotNumber", &slotNumber);
  bmp_cfg_db_close(cfg);
  if(ok == FALSE) slotNumber = 1;
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
#endif
  aud_cfg_db_get_int(cfg, configSectionString, "dithering_mode", &conf_dithering);
  aud_cfg_db_get_int(cfg, configSectionString, "latency_index", &conf_latency_index);
  aud_cfg_db_get_int(cfg, configSectionString, "zero_latency", &conf_rev_zl);
  gboolean ok = aud_cfg_db_get_int(cfg, configSectionString, "slotNumber", &slotNumber);
  aud_cfg_db_close(cfg);
  if(ok == FALSE) slotNumber = 1;
#endif
#endif
  fprintf(stderr, "Impulser2: init: %d slot(s)\n", slotNumber);
  
  // load Slot Config and autoload files
  slotVector->clear();
  currentSlotVector->clear();
  reverbVector->clear();
  for(int i = 1;i <= slotNumber;i ++)
    {
      SlotConfiguration slotC;
      slot_load(&slotC, i);
      if(store_inf(slotC.filename.c_str()) != 0)
	sprintf(inf, "(not loaded)");
      slotC.inf = inf;
      slotVector->push_back(slotC);
      set_rt_reverb(reverbVector->push_back(presetIRModelValue[conf_rev_zl]), &slotC);
    }
  StreamFs = 0;

  pthread_mutex_unlock(&plugin_mutex);
}

static void cleanup(void)
{
#ifdef DEBUG
  fprintf(stderr, "Impulser2: cleanup()\n");
#endif

  pthread_mutex_lock(&plugin_mutex);
  plugin_available = false;

  if(conf_rev_dialog != NULL)
    gtk_widget_destroy(GTK_WIDGET(conf_rev_dialog));
  fprintf(stderr, "Impulser2: WARNING: cleanup during play is not supported!!\n");
  if(validModel == true)
    {
      fprintf(stderr, "Impulser2: cleanup: vector %d, cvector %d\n",
	      (int)slotVector->size(), (int)currentSlotVector->size());
      reverbVector->clear();
      StreamFs = 0;
      delete slotVector;
      delete currentSlotVector;
      delete reverbVector;
      validModel = false;
    }
  
  if(gdither_on == FALSE)
    gdither_free(pdither);
  fprintf(stderr, "Impulser2: cleanup: done.\n");

  pthread_mutex_unlock(&plugin_mutex);

#ifndef __GNUC__
  plugin_fini();
#endif
}

static int validNumber = 0;
static void mod_samples_f(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate)
{
  if(length <= 0) return;
  if(validModel != true) fprintf(stderr, "Impulser2: !validModel\n");
  if(pthread_mutex_trylock(&plugin_mutex) == EBUSY) return;
  if(plugin_available != true)
    {
      pthread_mutex_unlock(&plugin_mutex);
      return;
    }

  if((int)reverbVector->size() != slotNumber)
    {
      fprintf(stderr, "Impulser2: mod_samples: Slot %d -> %d\n", (int)reverbVector->size(), slotNumber);
      if((int)reverbVector->size() < slotNumber) // increase slot
	{
	  SlotConfiguration slotC;
	  slot_init(&slotC);
	  while(1)
	    {
	      if((int)reverbVector->size() == slotNumber) break;
	      IRBASE * model = reverbVector->push_back(presetIRModelValue[conf_rev_zl]);
	      if(reverbVector->size() <= slotVector->size())
		set_rt_reverb(model, &(*slotVector)[reverbVector->size()-1]);
	      currentSlotVector->push_back(slotC);
	    }
	}
      else // decrease slot
	{
	  while(1)
	    {
	      if((int)reverbVector->size() == slotNumber) break;
	      reverbVector->pop_back();
	      currentSlotVector->pop_back();
	    }
	}
    }
  
  // reset all if srate or latency have been changed
  if(StreamFs != srate||latencyIndex != conf_latency_index)
    {
      fprintf(stderr, "Impulser2: mod_samples: Fs %d -> %d IRM %d\n", StreamFs, srate, conf_rev_zl);
      StreamFs = srate;
      latencyIndex = conf_latency_index;
      SlotConfiguration slotC;
      slot_init(&slotC);
      currentSlotVector->clear();
      for(int i = 0;i < slotNumber;i ++)
	{
	  currentSlotVector->push_back(slotC);
	  reverbVector->assign(i, presetIRModelValue[conf_rev_zl]);
	  set_rt_reverb(reverbVector->at(i), &(*slotVector)[i]);
	}
      fprintf(stderr, "Impulser2: mod_samples: vector %d, cvector %d\n",
	      (int)slotVector->size(), (int)currentSlotVector->size());
    }
  
  for(int i = 0;i < (int)reverbVector->size();i ++)
    {
      if(reverbVector->size() > slotVector->size()) return;
      // changed stretch/limit reload
      if((*currentSlotVector)[i].stretch != (*slotVector)[i].stretch||
	 (*currentSlotVector)[i].limit != (*slotVector)[i].limit||
	 strcmp((*currentSlotVector)[i].filename.c_str(), (*slotVector)[i].filename.c_str()) != 0)
	{
	  fprintf(stderr, "Impulser2: mod_samples: typeid=%s\n", typeid(*(*reverbVector)[i]).name());
	  (*currentSlotVector)[i].stretch = (*slotVector)[i].stretch;
	  (*currentSlotVector)[i].limit = (*slotVector)[i].limit;
	  if(latencyIndex >= presetLatencyMax) conf_latency_index = latencyIndex = 0;
	  if(typeid(*(*reverbVector)[i]) == typeid(IRMODEL2))
	    {
	      fprintf(stderr, "Impulser2: mod_samples: irmodel2 %ld\n", presetLatencyValue[latencyIndex]);
	      dynamic_cast<IRMODEL2*>((*reverbVector)[i])->setFragmentSize(presetLatencyValue[latencyIndex]);
	    }
	  if(typeid(*(*reverbVector)[i]) == typeid(IRMODEL2ZL))
	    {
	      fprintf(stderr, "Impulser2: mod_samples: irmodel2zl %ld\n", presetLatencyValue[latencyIndex]);
	      dynamic_cast<IRMODEL2ZL*>((*reverbVector)[i])->setFragmentSize(presetLatencyValue[latencyIndex]);
	    }
	  if(typeid(*(*reverbVector)[i]) == typeid(IRMODEL3))
	    {
	      fprintf(stderr, "Impulser2: mod_samples: irmodel3 %ld %ld\n",
		      presetLatencyValue1[latencyIndex], presetLatencyValue2[latencyIndex]);
	      dynamic_cast<IRMODEL3*>((*reverbVector)[i])->setFragmentSize(presetLatencyValue1[latencyIndex],
									   presetLatencyValue2[latencyIndex]);
	    }
	  CFILELOADER fileLoader;
	  double l_stretch = std::pow(static_cast<double>(std::sqrt(2)), static_cast<double>((*slotVector)[i].stretch));
	  int ret = fileLoader.load((*slotVector)[i].filename.c_str(), srate,
				    l_stretch, (*slotVector)[i].limit, SRC_SINC_BEST_QUALITY);
	  
	  if(ret == 0)
	    {
	      (*reverbVector)[i]->loadImpulse(fileLoader.out.L, fileLoader.out.R, fileLoader.out.getsize());
	      (*currentSlotVector)[i].filename = (*slotVector)[i].filename;
	      (*currentSlotVector)[i].valid = 1;
	      fprintf(stderr, "Impulser2: mod_samples: Slot[%d] \"%s\"(%ld)\n",
		      i, (*slotVector)[i].filename.c_str(), (*reverbVector)[i]->getSampleSize());
	    }
	  else
	    {
	      fprintf(stderr, "Impulser2: mod_samples: Slot[%d] IR load fail! ret=%d ", i, ret);
	      fprintf(stderr, "<%s>\n", fileLoader.errstr());
	      (*currentSlotVector)[i].valid = 0;
	    }
	}

      // delay
      long iDelay = (int)((float)StreamFs*(*slotVector)[i].idelay/1000.0f);
      if((*reverbVector)[i]->getInitialDelay() != iDelay)
	{
	  fprintf(stderr, "Impulser2: mod_samples: InitialDelay[%d] %ld -> %ld\n",
		  i, (*reverbVector)[i]->getInitialDelay(), iDelay);
	  (*reverbVector)[i]->setInitialDelay(iDelay);
	}
      
      // slot
      if((*slotVector)[i].i1o2_index < presetSlotModeMax&&(*slotVector)[i].i1o2_index >= 0)
	{
	  if((*currentSlotVector)[i].i1o2_index != presetSlotModeValue[(*slotVector)[i].i1o2_index])
	    {
	      (*currentSlotVector)[i].i1o2_index = presetSlotModeValue[(*slotVector)[i].i1o2_index];
	      fprintf(stderr, "Impulser2: mod_samples: conf_i1o2[%d] -> %d\n",
		      i, (*currentSlotVector)[i].i1o2_index);
	    }
	}
    }
  
  validNumber = 0;
  for(int i = 0;i < (int)reverbVector->size();i ++)
    {
      if((*currentSlotVector)[i].valid == 1&&(int)slotVector->size() > i)
	{
	  unsigned options = FV3_IR_DEFAULT;
	  if((*currentSlotVector)[i].i1o2_index == 1)
	    options |= FV3_IR_MONO2STEREO;
	  if((*currentSlotVector)[i].i1o2_index == 3)
	    options |= FV3_IR_SWAP_LR;
	  if((*slotVector)[i].wet <= MIN_DB)
	    options |= FV3_IR_MUTE_WET;
	  if((*slotVector)[0].dry <= MIN_DB||i > 0)
	    options |= FV3_IR_MUTE_DRY;
	  if((*slotVector)[i].lpf <= 0.0&&(*slotVector)[i].hpf <= 0.0)
	    options |= FV3_IR_SKIP_FILTER;
	  if((int)reverbVector->size() > i)
	    {
	      if((*reverbVector)[i]->getSampleSize() > 0)
		{
		  if(i == 0)
		    (*reverbVector)[i]->processreplace(iL,iR,oL,oR,length,options);
		  else
		    (*reverbVector)[i]->processreplace(iL,iR,oL,oR,length,options|FV3_IR_SKIP_INIT);
		  validNumber ++;
		}
	    }
	}
    }  
  pthread_mutex_unlock(&plugin_mutex);
}

static SLOTP origLR, orig, reverb;
#if __AUDACIOUS_PLUGIN_API__ >= 13
static void mod_samples_d(gfloat * LR, gint samples, gint srate)
{
  if(orig.getsize() < samples)
    {
      orig.alloc(samples, 2);
      reverb.alloc(samples, 2);
    }
#ifdef PLUGDOUBLE
  for(int tmpi = 0;tmpi < samples;tmpi ++)
    {
      orig.L[tmpi] = LR[tmpi*2+0];
      orig.R[tmpi] = LR[tmpi*2+1];
    }
#else
  fv3::splitChannelsV(2, samples, LR, orig.L, orig.R);
#endif
  mod_samples_f(orig.L,orig.R,reverb.L,reverb.R,samples,srate);
  if(validNumber <= 0) return;
  if(next_dithering != conf_dithering||gdither_on == FALSE)
    {
      fprintf(stderr, "Impulser2: mod_samples: Dither [%d] -> [%d]\n", conf_dithering, next_dithering);
      if(gdither_on == FALSE) gdither_free(pdither);
      pdither = gdither_new(presetDitherValue[next_dithering], 2, GDitherFloat, 16);
      gdither_on = TRUE;
      conf_dithering = next_dithering;
    }
#ifdef PLUGDOUBLE
  gdither_run(pdither, 0, samples, reverb.L, LR);
  gdither_run(pdither, 1, samples, reverb.R, LR);
#else
  gdither_runf(pdither, 0, samples, reverb.L, LR);
  gdither_runf(pdither, 1, samples, reverb.R, LR);
#endif
}
#else
static int mod_samples(gpointer * d, gint length, AFormat afmt, gint srate, gint nch)
{
  if((!(afmt == FMT_S16_NE || (afmt == FMT_S16_LE && G_BYTE_ORDER == G_LITTLE_ENDIAN)||
	(afmt == FMT_S16_BE && G_BYTE_ORDER == G_BIG_ENDIAN)))||nch != 2||length <= 0) return length;
  short int * data = (gint16*)*d;
  int len = length/sizeof(short int);
  if(origLR.getsize() < len)
    {
      origLR.alloc(len, 1);
      orig.alloc(len/2, 2);
      reverb.alloc(len/2, 2);
    }
#ifdef PLUGDOUBLE
  src_short_to_double_array(data, origLR.L, len);
#else
  src_short_to_float_array(data, origLR.L, len);
#endif
  fv3::splitChannelsV(2, len/2, origLR.L, orig.L, orig.R);
  mod_samples_f(orig.L,orig.R,reverb.L,reverb.R,len/2,srate);
  if(validNumber <= 0) return length;
  if(next_dithering != conf_dithering||gdither_on == FALSE)
    {
      fprintf(stderr, "Impulser2: mod_samples: Dither [%d] -> [%d]\n", conf_dithering, next_dithering);
      if(gdither_on == FALSE) gdither_free(pdither);
      pdither = gdither_new(presetDitherValue[next_dithering], 2, GDither16bit, 16);
      gdither_on = TRUE;
      conf_dithering = next_dithering;
    }
#ifdef PLUGDOUBLE
  gdither_run(pdither, 0, len/2, reverb.L, data);
  gdither_run(pdither, 1, len/2, reverb.R, data);
#else
  gdither_runf(pdither, 0, len/2, reverb.L, data);
  gdither_runf(pdither, 1, len/2, reverb.R, data);
#endif
  if(validModel != true) fprintf(stderr, "Impulser2: !validModel\n");
  return length;
}
static void query_format(AFormat * fmt, gint * rate, gint * nch)
{
  if (!(*fmt == FMT_S16_NE || (*fmt == FMT_S16_LE && G_BYTE_ORDER == G_LITTLE_ENDIAN) ||
	(*fmt == FMT_S16_BE && G_BYTE_ORDER == G_BIG_ENDIAN))) *fmt = FMT_S16_NE;
  if (*nch != 2) *nch = 2;
}
#endif

#ifndef AUDACIOUS140
static EffectPlugin ep = {
  NULL, NULL, (char*)productString,
  init, cleanup, about, configure, mod_samples, query_format,
};
extern "C" EffectPlugin *get_eplugin_info(void)
{
#ifdef DEBUG
  fprintf(stderr, "freeverb3.cpp: get_eplugin_info()\n");
#endif
  init();
  return &ep;
}
#else
#if __AUDACIOUS_PLUGIN_API__ >= 13
static gint impulser2_decoder_to_output_time (gint time){ return time; }
static gint impulser2_output_to_decoder_time (gint time){ return time; }
static gint plugin_rate = 0, plugin_ch = 0;
static void impulser2_start(gint * channels, gint * rate)
{
  fprintf(stderr, "Impulser2: start: Ch %d Fs %d\n", *channels, *rate);
  plugin_rate = *rate;
  plugin_ch = *channels;
}
static void impulser2_process(gfloat ** data, gint * samples)
{
  if(plugin_rate <= 0||plugin_ch != 2) return;
  mod_samples_d(*data, *samples/plugin_ch, plugin_rate);
}
static void impulser2_flush()
{
  fprintf(stderr, "Impulser2: flush:\n");
}
static void impulser2_finish(gfloat ** data, gint * samples)
{
  fprintf(stderr, "Impulser2: finish\n");
  impulser2_process(data,samples);
}
#endif

static EffectPlugin epe = {
  NULL, /* handle */
  NULL, /* filename */
  (gchar*)productString, /* description */
  init,
  cleanup,
  about,
  configure,

#if __AUDACIOUS_PLUGIN_API__ >= 10
  NULL, /* settings */
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
  NULL, /* sendmsg */
#endif

#if __AUDACIOUS_PLUGIN_API__ < 16
  FALSE, /* enabled */
#endif

#if __AUDACIOUS_PLUGIN_API__ < 16
# if __AUDACIOUS_PLUGIN_API__ >= 13
  NULL, NULL, /* mod_samples, query_format */
# else
  mod_samples, query_format,
# endif
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 13
  impulser2_start,
  impulser2_process,
  impulser2_flush,
  impulser2_finish,
  impulser2_decoder_to_output_time,
  impulser2_output_to_decoder_time,
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
  0, /* order */
  TRUE, /* preserves_format */
#endif
};

static EffectPlugin *eplist[] = {&epe, NULL};
SIMPLE_EFFECT_PLUGIN(plugin, eplist);
#endif

