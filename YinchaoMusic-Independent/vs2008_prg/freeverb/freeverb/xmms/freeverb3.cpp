/*  NReverb XMMS plugin
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <freeverb/nrev.hpp>
#include <freeverb/nrevb.hpp>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>
#include "xmms_defs.h"

static const char *about_text =
  "Freeverb3 "VERSION"\n"
  "NReverb (v3F)\n"
  "XMMS/BMP/Audacious Plugin\n"
#ifdef PLUGDOUBLE
  "Double Precision Version\n"
#else
  "Single Precision Version\n"
#endif
#ifdef PLUGINIT
  "Self Plugin Init/Cleanup\n"
#endif
  "Copyright (C) 2006-2010 Teru KAMOGASHIRA\n"
  "http://freeverb3.sourceforge.net/";

static const char * productString =
  "Freeverb3 "VERSION" [NReverb(v3F)]";

#define DEFAULTMAXVFS 192000
#define DEFAULTFS 44100

#ifdef PLUGDOUBLE
static fv3::nrevb_ reverbm;
typedef fv3::slot_ SLOTP;
typedef double pfloat_t;
#else
static fv3::nrevb_f reverbm;
typedef fv3::slot_f SLOTP;
typedef float pfloat_t;
#endif

//static long converter_type = SRC_SINC_FASTEST;
static long converter_type = SRC_ZERO_ORDER_HOLD;

static int maxvfs;
static int currentmaxvfs;
static int currentfactor;
static int currentfs;

static float conf_wet, conf_roomsize, conf_feedback,
  conf_dry, conf_damp, conf_damp2, conf_damp3, conf_width,
  conf_idelay;
static long iDelay;
static long protectValue = 0;

static long presetN = 6;
static char presetV[][256] = {
  "Hall 1",
  "Hall 2",
  "Room 1",
  "Room 2",
  "Stadium",
  "Drum Chamber",};
static float presetI[][9] = {
  {2.05, -10.0, -2.0,  0.65, 0.3,  0.7,   0.43, 0.9, 20,},
  {3.05, -15.0, -2.0,  0.6,  0.85, 0.85,  0.10, 1.0, 30,},
  {1.0,  -10.0, -2.0,  0.7,  0.4,  0.2,   0.25, 0.9, 3,},
  {1.0,  -10.0, -2.0,  0.6,  0.9,  0.9,   0.10, 1.0, 0,},
  {2.98, -15.0, -2.0,  0.6,  0.21, 0.6,   0.05, 1.0, 40,},
  {1.07, -10.0, -2.0,  0.56,  0.9,  0.9,   0.2, 1.0, 10,},};

#ifdef PLUGINIT
static pthread_mutex_t plugin_mutex;
static gboolean plugin_available = false;
#endif

static void init_reverb(void)
{
  reverbm.setroomsize(2.05);
  reverbm.setwet(-10.0f);
  reverbm.setdry(-5.0f);
  reverbm.setfeedback(0.65);
  reverbm.setdamp(0.3);
  reverbm.setdamp2(0.7);
  reverbm.setdamp3(0.43);
  reverbm.setwidth(0.9);
  reverbm.mute();
  conf_idelay = 0.0f;
}

static void set_reverb(void)
{
  reverbm.setroomsize(conf_roomsize);
  reverbm.setwet(conf_wet);
  reverbm.setdry(conf_dry);
  reverbm.setfeedback(conf_feedback);
  reverbm.setdamp(conf_damp);
  reverbm.setdamp2(conf_damp2);
  reverbm.setdamp3(conf_damp3);
  reverbm.setwidth(conf_width);
#ifdef DEBUG
  reverbm.printconfig();
#endif
}

static void set_reverb(float conf[9])
{
  reverbm.setroomsize(conf[0]);
  reverbm.setwet(conf[1]);
  reverbm.setdry(conf[2]);
  reverbm.setfeedback(conf[3]);
  reverbm.setdamp(conf[4]);
  reverbm.setdamp2(conf[5]);
  reverbm.setdamp3(conf[6]);
  reverbm.setwidth(conf[7]);
  conf_idelay = conf[8];
  reverbm.mute();
}

static void get_reverb(void)
{
  conf_wet = reverbm.getwet();
  conf_dry = reverbm.getdry();
  conf_roomsize = reverbm.getroomsize();
  conf_feedback = reverbm.getfeedback();
  conf_damp = reverbm.getdamp();
  conf_damp2 = reverbm.getdamp2();
  conf_damp3 = reverbm.getdamp3();
  conf_width = reverbm.getwidth();
}

static void init(void);
static void cleanup(void);

// GUI

static GtkWidget *conf_dialog = NULL;
static GtkObject *conf_wet_adj, *conf_roomsize_adj, *conf_feedback_adj,
  *conf_dry_adj, *conf_damp_adj, *conf_damp2_adj, *conf_damp3_adj,
  *conf_width_adj, *conf_maxvfs_adj, *conf_idelay_adj;

static void write_config(void)
{
#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  xmms_cfg_write_int  (cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("maxvfs"), maxvfs);
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("wet"), reverbm.getwet());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("dry"), reverbm.getdry());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("roomsize"), reverbm.getroomsize());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("feedback"), reverbm.getfeedback());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp"), reverbm.getdamp());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp2"), reverbm.getdamp2());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp3"), reverbm.getdamp3());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("width"), reverbm.getwidth());
  xmms_cfg_write_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("idelay"), conf_idelay);
  xmms_cfg_write_default_file(cfg);
  xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  bmp_cfg_db_set_int  (cfg, "freeverb3_plugin", "maxvfs", maxvfs);
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "wet", reverbm.getwet());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "dry", reverbm.getdry());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "roomsize", reverbm.getroomsize());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "feedback", reverbm.getfeedback());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "damp", reverbm.getdamp());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "damp2", reverbm.getdamp2());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "damp3", reverbm.getdamp3());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "width", reverbm.getwidth());
  bmp_cfg_db_set_float(cfg, "freeverb3_plugin", "idelay", conf_idelay);
  bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
# endif
  aud_cfg_db_set_int  (cfg, "freeverb3_plugin", "maxvfs", maxvfs);
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "wet", reverbm.getwet());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "dry", reverbm.getdry());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "roomsize", reverbm.getroomsize());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "feedback", reverbm.getfeedback());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "damp", reverbm.getdamp());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "damp2", reverbm.getdamp2());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "damp3", reverbm.getdamp3());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "width", reverbm.getwidth());
  aud_cfg_db_set_float(cfg, "freeverb3_plugin", "idelay", conf_idelay);
  aud_cfg_db_close(cfg);
#endif
#endif
}

static GtkWidget *applyButton = NULL;
static void non_realtime_changes(void)
{
  if(applyButton != NULL)
    gtk_widget_set_sensitive(applyButton, TRUE);
}

static void apply_changes(void)
{
  if(applyButton != NULL) gtk_widget_set_sensitive(applyButton, FALSE);
  maxvfs = (int)GTK_ADJUSTMENT(conf_maxvfs_adj)->value*1000;
  conf_wet = GTK_ADJUSTMENT(conf_wet_adj)->value;
  conf_dry = GTK_ADJUSTMENT(conf_dry_adj)->value;
  conf_roomsize = GTK_ADJUSTMENT(conf_roomsize_adj)->value;
  conf_feedback = GTK_ADJUSTMENT(conf_feedback_adj)->value;
  conf_damp = GTK_ADJUSTMENT(conf_damp_adj)->value;
  conf_damp2 = GTK_ADJUSTMENT(conf_damp2_adj)->value;
  conf_damp3 = GTK_ADJUSTMENT(conf_damp3_adj)->value;
  conf_width = GTK_ADJUSTMENT(conf_width_adj)->value;
  conf_idelay = GTK_ADJUSTMENT(conf_idelay_adj)->value;
  set_reverb();
  write_config();
}

static void apply_realtime_changes(void)
{
  if(protectValue == 0)
    apply_changes();
}

static void change_values(void)
{
  gtk_adjustment_set_value((GtkAdjustment*)conf_wet_adj, conf_wet);
  gtk_adjustment_set_value((GtkAdjustment*)conf_dry_adj, conf_dry);
  gtk_adjustment_set_value((GtkAdjustment*)conf_roomsize_adj, conf_roomsize);
  gtk_adjustment_set_value((GtkAdjustment*)conf_feedback_adj, conf_feedback);
  gtk_adjustment_set_value((GtkAdjustment*)conf_damp_adj, conf_damp);
  gtk_adjustment_set_value((GtkAdjustment*)conf_damp2_adj, conf_damp2);
  gtk_adjustment_set_value((GtkAdjustment*)conf_damp3_adj, conf_damp3);
  gtk_adjustment_set_value((GtkAdjustment*)conf_width_adj, conf_width);
  gtk_adjustment_set_value((GtkAdjustment*)conf_idelay_adj, conf_idelay);
}

static void conf_ok_cb(GtkButton * button, gpointer data)
{
  apply_changes();
  gtk_widget_destroy(GTK_WIDGET(conf_dialog));
}

static void conf_cancel_cb(GtkButton * button, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(conf_dialog));
}

static void conf_apply_realtime_cb(GtkButton * button, gpointer data)
{
  apply_realtime_changes();
}

static void conf_non_realtime_cb(GtkButton * button, gpointer data)
{
  non_realtime_changes();
}

static void conf_apply_cb(GtkButton * button, gpointer data)
{
  apply_changes();
}

static void conf_setted()
{
  get_reverb();
  change_values();
  gtk_adjustment_set_value((GtkAdjustment*)conf_maxvfs_adj, DEFAULTMAXVFS/1000);
  maxvfs = DEFAULTMAXVFS;
  protectValue = 0;
  if(applyButton != NULL)
    gtk_widget_set_sensitive(applyButton, FALSE);
}

static void conf_preset_cb(GtkButton * button, gpointer data)
{
  std::fprintf(stderr, "freeverb3.cpp: preset %s(%d)\n",
	       presetV[GPOINTER_TO_INT(data)], GPOINTER_TO_INT(data));
  protectValue = 1;
  set_reverb(presetI[GPOINTER_TO_INT(data)]);
  conf_setted();
}

static void conf_default_cb(GtkButton * button, gpointer data)
{
  protectValue = 1;
  init_reverb();
  conf_setted();
}

static void configure(void)
{
  GtkWidget *button, *table, *label, *hscale, *bbox;
  GtkOptionMenu *omenu;
  if (conf_dialog != NULL)
    return;
  
  conf_dialog = gtk_dialog_new();
  gtk_signal_connect(GTK_OBJECT(conf_dialog), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &conf_dialog);
  gtk_window_set_title(GTK_WINDOW(conf_dialog), productString);

  conf_maxvfs_adj = gtk_adjustment_new((gfloat)maxvfs/1000,     0.0, 768+1.0, 1, 10, 1.0);
  conf_roomsize_adj = gtk_adjustment_new(conf_roomsize, 0.0, 15.0+1.0, 0.01, 0.1, 1.0);
  conf_wet_adj = gtk_adjustment_new(conf_wet,        -100.0,20.0+1.0, 0.01, 0.1, 1.0);
  conf_dry_adj = gtk_adjustment_new(conf_dry,        -100.0,20.0+1.0, 0.01, 0.1, 1.0);
  conf_feedback_adj = gtk_adjustment_new(conf_feedback, 0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_damp_adj = gtk_adjustment_new(conf_damp,         0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_damp2_adj = gtk_adjustment_new(conf_damp2,       0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_damp3_adj = gtk_adjustment_new(conf_damp3,       0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_width_adj = gtk_adjustment_new(conf_width,       0.0, 1.0+1.0, 0.01, 0.1, 1.0);
  conf_idelay_adj = gtk_adjustment_new(conf_idelay,  -500.0, 500.0+1.0, 0.01, 0.1, 1.0);

#define LABELS 10
  const char * labels[] = {"OverSamplingRate[kHz]",
			   "ReverbTime [s]",
			   "Wet [dB]", "Dry [dB]", "Feedback",
			   "Damp [LPF (air)]", "Damp2 [LPF (room)]", "Damp3 [HPF (air)]",
			   "Width", "Initial Delay[ms]",};
  
  table = gtk_table_new(2, LABELS+1, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(table), 10);
  gtk_container_set_border_width(GTK_CONTAINER(table), 10);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(conf_dialog)->vbox), table,
		     TRUE, TRUE, 10);
  gtk_widget_show(table);
  
  for(int i = 0;i < LABELS;i ++)
    {
#ifdef DEBUG
      std::fprintf(stderr, "[%d]%s\n", i, labels[i]);
#endif
      label = gtk_label_new(labels[i]);
      gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
      gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i+1, GTK_FILL, GTK_FILL, 0, 0);
      gtk_widget_show(label);
    }
  
  GtkObject * objects[] =
    {conf_maxvfs_adj, conf_roomsize_adj, conf_wet_adj, conf_dry_adj, conf_feedback_adj,
     conf_damp_adj, conf_damp2_adj, conf_damp3_adj, conf_width_adj, conf_idelay_adj};
  for(int i = 0;i < LABELS;i ++)
    {
#ifdef DEBUG
      std::fprintf(stderr, "[%d]\n", i);
#endif
      hscale = gtk_hscale_new(GTK_ADJUSTMENT(objects[i]));
      gtk_widget_set_usize(hscale, 400, 35);
      if(i == 0)
	gtk_scale_set_digits(GTK_SCALE(hscale), 0); // [Hz]
      else
	gtk_scale_set_digits(GTK_SCALE(hscale), 2); // under .
      gtk_table_attach_defaults(GTK_TABLE(table), hscale, 1, 2, i, i+1);
      gtk_widget_show(hscale);
    }
  int isRealTimeObject[] =
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 0};
  for(int i = 0;i < LABELS;i ++)
    {
      if(isRealTimeObject[i] == 1)
	gtk_signal_connect(GTK_OBJECT(objects[i]),
			   "value_changed", GTK_SIGNAL_FUNC(conf_apply_realtime_cb), NULL);
      else
	gtk_signal_connect(GTK_OBJECT(objects[i]),
			   "value_changed", GTK_SIGNAL_FUNC(conf_non_realtime_cb), NULL);
    }
  
  bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 2);
  gtk_box_pack_start(GTK_BOX((GTK_DIALOG(conf_dialog)->action_area)),
		     bbox, TRUE, TRUE, 0);
  
  button = gtk_button_new_with_label("Ok");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(conf_ok_cb), NULL);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label("Cancel");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(conf_cancel_cb), NULL);
  gtk_widget_show(button);
  
  applyButton = button = gtk_button_new_with_label("Apply");
  gtk_widget_set_sensitive(applyButton, FALSE);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(conf_apply_cb), NULL);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label("Default");
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(conf_default_cb), NULL);
  gtk_widget_show(button);

  // Preset
  omenu = (GtkOptionMenu*)gtk_option_menu_new();
  GtkWidget *menu = gtk_menu_new();
  for(int i = 0;i < presetN; i++)
    {
      GtkWidget * item = gtk_menu_item_new_with_label(presetV[i]);
      gtk_signal_connect(GTK_OBJECT(item), "activate",
			 GTK_SIGNAL_FUNC(conf_preset_cb),
			 GINT_TO_POINTER(i));
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu), item);
    }
  gtk_option_menu_remove_menu(omenu);
  gtk_option_menu_set_menu(omenu, menu);
  gtk_box_pack_start(GTK_BOX(bbox), (GtkWidget*)omenu, TRUE, TRUE, 0);
  gtk_widget_show((GtkWidget*)omenu);

  gtk_widget_show(bbox);
  gtk_window_set_position((GtkWindow*)conf_dialog, GTK_WIN_POS_CENTER);
  gtk_widget_show(conf_dialog);
}

// plugin functions

static void about(void)
{
  static GtkWidget *about_dialog = NULL;
  if (about_dialog != NULL) return;
#if __AUDACIOUS_PLUGIN_API__ >= 16
  audgui_simple_message(&about_dialog, GTK_MESSAGE_INFO, (gchar*)"About Plugin", (gchar*)about_text);
#else
  about_dialog =  _XMMS_DIALOG(const_cast<char*>("About Plugin"),const_cast<char*>(about_text),const_cast<char*>("Ok"),FALSE,NULL,NULL);
  gtk_signal_connect(GTK_OBJECT(about_dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_dialog);
#endif
}

static void init(void)
{
#ifdef DEBUG
  std::fprintf(stderr, "freeverb3.cpp: init()\n");
#endif

#ifdef PLUGINIT
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = true;
#endif
  
#ifdef XMMS
  ConfigFile * cfg = xmms_cfg_open_default_file();
  gboolean ok =
    xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("wet"), &conf_wet);
  xmms_cfg_read_int  (cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("maxvfs"), &maxvfs);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("dry"), &conf_dry);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("roomsize"), &conf_roomsize);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("feedback"), &conf_feedback);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp"), &conf_damp);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp2"), &conf_damp2);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("damp3"), &conf_damp3);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("width"), &conf_width);
  xmms_cfg_read_float(cfg, const_cast<char*>("freeverb3_plugin"), const_cast<char*>("idelay"), &conf_idelay);
  xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
  ConfigDb * cfg = bmp_cfg_db_open();
  gboolean ok = 
    bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "wet", &conf_wet);
  bmp_cfg_db_get_int(cfg, "freeverb3_plugin", "maxvfs", &maxvfs);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "dry", &conf_dry);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "roomsize", &conf_roomsize);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "feedback", &conf_feedback);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "damp", &conf_damp);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "damp2", &conf_damp2);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "damp3", &conf_damp3);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "width", &conf_width);
  bmp_cfg_db_get_float(cfg, "freeverb3_plugin", "idelay", &conf_idelay);
  bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
  ConfigDb * cfg = aud_cfg_db_open();
# else
  mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
# endif
  gboolean ok = 
    aud_cfg_db_get_float(cfg, "freeverb3_plugin", "wet", &conf_wet);
  aud_cfg_db_get_int(cfg, "freeverb3_plugin", "maxvfs", &maxvfs);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "dry", &conf_dry);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "roomsize", &conf_roomsize);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "feedback", &conf_feedback);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "damp", &conf_damp);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "damp2", &conf_damp2);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "damp3", &conf_damp3);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "width", &conf_width);
  aud_cfg_db_get_float(cfg, "freeverb3_plugin", "idelay", &conf_idelay);
  aud_cfg_db_close(cfg);
#endif
#endif
  
  if(ok == FALSE)
    currentmaxvfs = maxvfs = DEFAULTMAXVFS;
  else
    currentmaxvfs = maxvfs;
  currentfs = DEFAULTFS;
  currentfactor = maxvfs/currentfs;
  reverbm.setCurrentFs(currentfs);
  reverbm.setOverSamplingFactor(currentfactor, converter_type);
  std::fprintf(stderr, "freeverb3.cpp: fs = %d[Hz] x %ld\n",
	  currentfs, reverbm.getOverSamplingFactor());

  if(ok == FALSE)
    init_reverb();
  else
    set_reverb();
  get_reverb();

#ifdef PLUGINIT
  pthread_mutex_unlock(&plugin_mutex);
#endif
}

static void cleanup(void)
{
#ifdef DEBUG
  std::fprintf(stderr, "freeverb3.cpp: cleanup()\n");
#endif

#ifdef PLUGINIT
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = false;
#endif

  if(conf_dialog != NULL)
    gtk_widget_destroy(GTK_WIDGET(conf_dialog));
  
  std::fprintf(stderr, "freeverb3.cpp: WARNING: cleanup during play is not supported!!\n");

#ifdef PLUGINIT
  pthread_mutex_unlock(&plugin_mutex);
#endif
}

static void mod_samples(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate)
{  
#ifdef PLUGINIT
  if(pthread_mutex_trylock(&plugin_mutex) == EBUSY) return length;
  if(plugin_available != true)
    {
      pthread_mutex_unlock(&plugin_mutex);
      return length;
    }
#endif
  
  if(currentfs != srate||currentmaxvfs != maxvfs)
    {
      currentfs = srate;
      currentmaxvfs = maxvfs;
      currentfactor = maxvfs/currentfs;
      if(currentfactor < 1)
	currentfactor = 1;
      reverbm.setOverSamplingFactor(currentfactor, converter_type);
      reverbm.setCurrentFs(currentfs);
      set_reverb();
      std::fprintf(stderr, "freeverb3.cpp: resetting fs = %d[Hz] x %ld\n", currentfs, reverbm.getOverSamplingFactor());
#ifdef DEBUG
      reverbm.printconfig();
#endif
    }

  iDelay = (int)((float)currentfs*conf_idelay/1000.0f);
  if(reverbm.getInitialDelay() != iDelay)
    {
      std::fprintf(stderr, "freeverb3.cpp: IDelay %ld -> %ld\n", reverbm.getInitialDelay(), iDelay);
      reverbm.setInitialDelay(iDelay);
    }
  reverbm.processreplace(iL,iR,oL,oR,length);
#ifdef PLUGINIT
  pthread_mutex_unlock(&plugin_mutex);
#endif
}

SLOTP origLR, orig, reverb;
#if __AUDACIOUS_PLUGIN_API__ >= 13
static void mod_samples(gfloat * LR, gint samples, gint srate)
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
  mod_samples(orig.L,orig.R,reverb.L,reverb.R,samples,srate);
#ifdef PLUGDOUBLE
  for(int tmpi = 0;tmpi < samples;tmpi ++)
        {
          LR[tmpi*2+0] = reverb.L[tmpi];
          LR[tmpi*2+1] = reverb.R[tmpi];
        }
#else
  fv3::mergeChannelsV(2, samples, LR, reverb.L, reverb.R);
#endif
}
#else
static int mod_samples(gpointer * d, gint length, AFormat afmt, gint srate, gint nch)
{
  if((!(afmt == FMT_S16_NE||(afmt == FMT_S16_LE && G_BYTE_ORDER == G_LITTLE_ENDIAN)||
	(afmt == FMT_S16_BE && G_BYTE_ORDER == G_BIG_ENDIAN)))||nch != 2){ return length; }
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
  mod_samples(orig.L,orig.R,reverb.L,reverb.R,len/2,srate);
  fv3::mergeChannelsV(2, len/2, origLR.L, reverb.L, reverb.R);
#ifdef PLUGDOUBLE
  src_double_to_short_array(origLR.L, data, len);
#else
  src_float_to_short_array(origLR.L, data, len);
#endif
  return length;
}
static void query_format(AFormat * fmt, gint * rate, gint * nch)
{
  if (!(*fmt == FMT_S16_NE ||(*fmt == FMT_S16_LE && G_BYTE_ORDER == G_LITTLE_ENDIAN) ||
	(*fmt == FMT_S16_BE && G_BYTE_ORDER == G_BIG_ENDIAN))) *fmt = FMT_S16_NE;
  if (*nch != 2) *nch = 2;
}
#endif

#ifndef AUDACIOUS140
static EffectPlugin ep = {
  NULL, NULL, (char*)productString,
  init, cleanup, about, configure, mod_samples, NULL,
};

extern "C" EffectPlugin *get_eplugin_info(void)
{
#ifdef DEBUG
  std::fprintf(stderr, "freeverb3.cpp: get_eplugin_info()\n");
#endif
  init();
  return &ep;
}

#else

#if __AUDACIOUS_PLUGIN_API__ >= 13
static gint decoder_to_output_time (gint time){ return time; }
static gint output_to_decoder_time (gint time){ return time; }
gint plugin_rate = 0, plugin_ch = 0;
static void freeverb3_start(gint * channels, gint * rate)
{
  fprintf(stderr, "Freeverb3: start: Ch %d Fs %d\n", *channels, *rate);
  plugin_rate = *rate;
  plugin_ch = *channels;
}
static void freeverb3_process(gfloat ** data, gint * samples)
{
  if(plugin_rate <= 0||plugin_ch != 2) return;
  mod_samples(*data, *samples/plugin_ch, plugin_rate);
}
static void freeverb3_flush()
{
  fprintf(stderr, "Freeverb3: flush:\n");
}
static void freeverb3_finish(gfloat ** data, gint * samples)
{
  fprintf(stderr, "Freeverb3: finish\n");
  freeverb3_process(data,samples);
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
  freeverb3_start,
  freeverb3_process,
  freeverb3_flush,
  freeverb3_finish,
  decoder_to_output_time,
  output_to_decoder_time,
#endif

#if __AUDACIOUS_PLUGIN_API__ >= 16
  0, /* order */
  TRUE, /* preserves_format */
#endif
};

static EffectPlugin *eplist[] = {&epe, NULL};
SIMPLE_EFFECT_PLUGIN(plugin, eplist);
#endif

#ifdef PLUGINIT
static void __attribute__ ((constructor)) plugin_init(void)
{
  std::fprintf(stderr, "freeverb3.cpp: plugin_init()\n");
  pthread_mutex_init(&plugin_mutex, NULL);
}

static void __attribute__ ((destructor)) plugin_fini(void)
{
  std::fprintf(stderr, "freeverb3.cpp: plugin_fini()\n");
  pthread_mutex_destroy(&plugin_mutex);
}
#endif
