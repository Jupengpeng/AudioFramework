/*  XMMS plugin
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <string>
#include <map>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>
#include <libsamplerate2/samplerate2.h>

#include "xmms_defs.h"

#define DELIMITER  "/"

#ifdef PLUGDOUBLE
typedef double pfloat_t;
typedef fv3::slot_ SLOTP;
#else
typedef float pfloat_t;
typedef fv3::slot_f SLOTP;
#endif

enum { kFloat, kLong, kBool, kSelect, };
typedef void (*callbackFloat)(pfloat_t);
typedef void (*callbackLong)(long);
typedef void (*callbackBool)(gboolean);
typedef void (*callbackSelect)(long);
typedef struct
{
  void * self;
  std::string parameterDisplayName, parameterConfigName, parameterLabel, parameterSelect;
  pfloat_t minParam, maxParam;
  long displayDigit;
  pfloat_t valueF, defaultValueF;
  long valueL, defaultValueL;
  gboolean valueB, defaultValueB, isRTParameter, needNRTCall;
  unsigned typeOfParameter;
  void * callBack;
  GtkObject * gtkAdj;
  std::vector<std::string> menuStrings;
  std::map<GtkObject*,unsigned> selectMenu;
} PluginParameter;
typedef struct
{
  const char *parameterDisplayName, *parameterConfigName, *parameterLabel, *parameterSelect;
  const pfloat_t minParam, maxParam; const long displayDigit;
  const pfloat_t defaultValueF;
  const long defaultValueL;
  const gboolean defaultValueB, isRTParameter, needNRTCall;
  const unsigned typeOfParameter;
  void * callBack;
} PluginParameterTable;

namespace fv3
{
  class libxmmsplugin
  {
  public:
    libxmmsplugin(const PluginParameterTable * confTable, const unsigned tableSize,
		  const char * aboutStr, const char * productStr, const char * configSectionStr)
    {
      this->aboutString = aboutStr;
      this->productString = productStr;
      this->configSectionString = configSectionStr;
      initConfTable(confTable, tableSize);
      for(unsigned i = 0;i < ParamsV.size();i ++)
	{
	  PluginParameter * pp = &ParamsV[i];
	  gboolean result = readConfig(pp, configSectionString);
	  if(result == false) setDefault(pp);
	  if(pp->isRTParameter == true)
	    execCallBack(pp);
	  else
	    pp->needNRTCall = true;
	}
      conf_dialog = NULL;
      _mod_samples = NULL;
      plugin_rate = plugin_ch = 0;
    }
    
    ~libxmmsplugin()
    {
      if(conf_dialog != NULL) gtk_widget_destroy(GTK_WIDGET(conf_dialog));
    }

    void registerModSamples(void (*_vf)(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate))
    {
      _mod_samples = _vf;
    }

#if __AUDACIOUS_PLUGIN_API__ >= 13
    void mod_samples(gfloat * LR, gint samples, gint srate)
    {
      if(_mod_samples == NULL) return;
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
      _mod_samples(orig.L,orig.R,reverb.L,reverb.R,samples,srate);
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

    void start(gint * channels, gint * rate)
    {
      fprintf(stderr, "libxmmsplugin<%s>: start: Ch %d Fs %d\n", configSectionString, *channels, *rate);
      plugin_rate = *rate; plugin_ch = *channels;
    }

    void process(gfloat ** data, gint * samples)
    {
      if(plugin_rate <= 0||plugin_ch != 2) return;
      mod_samples(*data, *samples/plugin_ch, plugin_rate);
    }

    void flush()
    {
      fprintf(stderr, "libxmmsplugin<%s>: flush:\n", configSectionString);
    }
    
    void finish(gfloat ** data, gint * samples)
    {
      fprintf(stderr, "libxmmsplugin<%s>: finish:\n", configSectionString);
      process(data,samples);
    }
#else
    int mod_samples(gpointer * d, gint length, AFormat afmt, gint srate, gint nch)
    {
      if(_mod_samples == NULL) return 0;
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
      _mod_samples(orig.L,orig.R,reverb.L,reverb.R,len/2,srate);
      fv3::mergeChannelsV(2, len/2, origLR.L, reverb.L, reverb.R);
#ifdef PLUGDOUBLE
      src_double_to_short_array(origLR.L, data, len);
#else
      src_float_to_short_array(origLR.L, data, len);
#endif
      return length;
    }
    void query_format(AFormat * fmt, gint * rate, gint * nch)
    {
      if (!(*fmt == FMT_S16_NE ||(*fmt == FMT_S16_LE && G_BYTE_ORDER == G_LITTLE_ENDIAN) ||
	    (*fmt == FMT_S16_BE && G_BYTE_ORDER == G_BIG_ENDIAN))) *fmt = FMT_S16_NE;
      if (*nch != 2) *nch = 2;
    }
#endif
    
    gboolean callNRTParameters()
    {
      gboolean ret = false;
      for(unsigned i = 0;i < ParamsV.size();i ++)
	{
	  PluginParameter * pp = &ParamsV[i];
	  if(pp->isRTParameter == false&&pp->needNRTCall == true)
	    {
	      pp->needNRTCall = false;
	      execCallBack(pp);
	      ret = true;
	    }
	}
      return ret;
    }

    void about(void)
    {
      static GtkWidget *about_dialog = NULL;
      if (about_dialog != NULL) return;
#if __AUDACIOUS_PLUGIN_API__ >= 16
      audgui_simple_message(&about_dialog, GTK_MESSAGE_INFO, (gchar*)"About Plugin", (gchar*)aboutString);
#else
      about_dialog = _XMMS_DIALOG((gchar*)"About Plugin",(gchar*)aboutString,(gchar*)"Ok",FALSE,NULL,NULL);
      gtk_signal_connect(GTK_OBJECT(about_dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_dialog);
#endif
    }
    
    void configure(void)
    {
      if (conf_dialog != NULL) return;
      
      conf_dialog = gtk_dialog_new();
      gtk_signal_connect(GTK_OBJECT(conf_dialog), "destroy", GTK_SIGNAL_FUNC(gtk_widget_destroyed), &conf_dialog);
      gtk_window_set_title(GTK_WINDOW(conf_dialog), productString);
      gtk_window_set_default_size(GTK_WINDOW(conf_dialog), 800, 600);
#ifdef AUDACIOUS
      gtk_widget_set_size_request(conf_dialog, 800, 600);
#endif
      
      GtkWidget * scrolledWindow = gtk_scrolled_window_new (NULL, NULL);
      gtk_container_set_border_width(GTK_CONTAINER (scrolledWindow), 10);
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
      gtk_widget_show(scrolledWindow);
      
      gtk_box_pack_start(GTK_BOX(GTK_DIALOG(conf_dialog)->vbox), scrolledWindow, TRUE, TRUE, 0);
      
      GtkWidget * table = gtk_table_new(ParamsV.size()+1, 5, FALSE);
      gtk_table_set_col_spacings(GTK_TABLE(table), 1);
      gtk_container_set_border_width(GTK_CONTAINER(table), 11);
      gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledWindow), table);
      gtk_widget_show(table);
      
      // create left labels
      for(unsigned i = 0;i < ParamsV.size();i ++)
	{
	  GtkWidget * label = createGUILabel(&ParamsV[i]);
	  gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
	  gtk_table_attach(GTK_TABLE(table),label,0,1,i,i+1,GTK_FILL,GTK_FILL,0,0);
	  gtk_widget_show(label);
	}
      
      // create adjustment/checkbox/menu
      for(unsigned i = 0;i < ParamsV.size();i ++)
	{
	  GtkWidget * object = createGUIObject(&ParamsV[i]);
	  gtk_table_attach_defaults(GTK_TABLE(table),object,1,2,i,i+1);
	  gtk_widget_show(object);
	  setGUIObjectValue(&ParamsV[i]);
	}
      
      // register gui callback
      for(unsigned i = 0;i < ParamsV.size();i ++) registerGUICallback(ParamsV[i].gtkAdj, &ParamsV[i]);
      
      GtkWidget * bbox = gtk_hbutton_box_new();
      gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
      gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 2);
      gtk_box_pack_start(GTK_BOX((GTK_DIALOG(conf_dialog)->action_area)), bbox, TRUE, TRUE, 0);
      
      GtkWidget * button = gtk_button_new_with_label("Save");
      GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
      gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
      gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_save_cb), this);
      gtk_widget_grab_default(button);
      gtk_widget_show(button);
      
      button = gtk_button_new_with_label("Close");
      GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
      gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
      gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_close_cb), this);
      gtk_widget_show(button);
      GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
      
      button = gtk_button_new_with_label("Reload");
      GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
      gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
      gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_reload_cb), this);
      gtk_widget_show(button);
      
      button = gtk_button_new_with_label("Default");
      GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
      gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 0);
      gtk_signal_connect(GTK_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(conf_default_cb), this);
      gtk_widget_show(button);
      gtk_widget_show(bbox);
      
      gtk_widget_show(conf_dialog);
      
      gtk_window_set_position(GTK_WINDOW(conf_dialog), GTK_WIN_POS_CENTER); 
      gtk_widget_show_all(conf_dialog);
#ifdef AUDACIOUS
      gtk_window_present(GTK_WINDOW(conf_dialog));
#endif
    }
    
  private:    
    void initConfTable(const PluginParameterTable * confTable, const unsigned tableSize)
    {
      ParamsV.clear();
      for(unsigned i = 0;i < tableSize;i ++)
	{
	  PluginParameter pp;
	  const PluginParameterTable * conft = confTable+i;
	  pp.parameterDisplayName = conft->parameterDisplayName;
	  pp.parameterConfigName = conft->parameterConfigName;
	  pp.parameterLabel = conft->parameterLabel;
	  pp.parameterSelect = conft->parameterSelect;
	  pp.minParam = conft->minParam; pp.maxParam = conft->maxParam;
	  pp.displayDigit = conft->displayDigit;
	  pp.valueF = pp.defaultValueF = conft->defaultValueF;
	  pp.valueL = pp.defaultValueL = conft->defaultValueL;
	  pp.valueB = pp.defaultValueB = conft->defaultValueB;
	  pp.isRTParameter = conft->isRTParameter;
	  pp.needNRTCall = conft->needNRTCall;
	  pp.typeOfParameter = conft->typeOfParameter;
	  pp.callBack = conft->callBack;
	  ParamsV.push_back(pp);
	  ParamsV.at(i).self = &ParamsV.at(i);
	}
    }
    
    static long splitStringToVector(std::string * s, std::vector<std::string> * v, const char * delimiter)
    {
      long count;
      v->clear();
      char * token = strtok((char*)s->c_str(), delimiter);
      while (token != NULL)
	{
	  v->push_back(std::string(token));
	  token = strtok(NULL, DELIMITER);
	  count++;
	}
      return count;
    }

    static void setDefault(PluginParameter * pp)
    {
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  pp->valueF = pp->defaultValueF;
	  break;
	case kLong:
	case kSelect:
	  pp->valueL = pp->defaultValueL;
	  break;
	case kBool:
	  pp->valueB = pp->defaultValueB;
	  break;
	default:
	  break;
	}
    }
    
    static gboolean readConfig(PluginParameter * pp, const char * psName)
    {
      gboolean result = false;
#ifdef XMMS
      ConfigFile * cfg = xmms_cfg_open_default_file();
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  gfloat gf;
	  result = xmms_cfg_read_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gf);
	  pp->valueF = gf;
	  break;
	case kLong:
	case kSelect:
	  gint gi;
	  result = xmms_cfg_read_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gi);
	  pp->valueL = gi;
	  break;
	case kBool:
	  gboolean gb;
	  result = xmms_cfg_read_boolean(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gb);
	  pp->valueB = gb;
	  break;
	default:
	  break;
	}
      xmms_cfg_write_default_file(cfg);
      xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
      ConfigDb * cfg = bmp_cfg_db_open();
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  gfloat gf;
	  result = bmp_cfg_db_get_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gf);
	  pp->valueF = gf;
	  break;
	case kLong:
	case kSelect:
	  gint gi;
	  result = bmp_cfg_db_get_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gi);
	  pp->valueL = gi;
	  break;
	case kBool:
	  gboolean gb;
	  result = bmp_cfg_db_get_bool(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gb);
	  pp->valueB = gb;
	  break;
	default:
	  break;
	}
      bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
      ConfigDb * cfg = aud_cfg_db_open();
# else
      mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
# endif
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  gfloat gf;
	  result = aud_cfg_db_get_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gf);
	  pp->valueF = gf;
	  break;
	case kLong:
	case kSelect:
	  gint gi;
	  result = aud_cfg_db_get_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gi);
	  pp->valueL = gi;
	  break;
	case kBool:
	  gboolean gb;
	  result = aud_cfg_db_get_bool(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), &gb);
	  pp->valueB = gb;
	  break;
	default:
	  break;
	}
      aud_cfg_db_close(cfg);
#endif
#endif
      return result;
    }

    static void execCallBack(PluginParameter * pp)
    {
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  ((callbackFloat)pp->callBack)(pp->valueF);
	  break;
	case kLong:
	case kSelect:
	  ((callbackLong)pp->callBack)(pp->valueL);
	  break;
	case kBool:
	  ((callbackBool)pp->callBack)(pp->valueB);
	  break;
	default:
	  break;
	}
    }
    
    static void writeConfig(PluginParameter * pp, const char * psName)
    {
#ifdef XMMS
      ConfigFile * cfg = xmms_cfg_open_default_file();
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  xmms_cfg_write_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueF);
	  break;
	case kLong:
	case kSelect:
	  xmms_cfg_write_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueL);
	  break;
	case kBool:
	  xmms_cfg_write_boolean(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueB);
	  break;
	default:
	  break;
	}
      xmms_cfg_write_default_file(cfg);
      xmms_cfg_free(cfg);
#endif
#ifdef AUDACIOUS
#ifndef AUDACIOUS140
      ConfigDb * cfg = bmp_cfg_db_open();
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  bmp_cfg_db_set_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueF);
	  break;
	case kLong:
	case kSelect:
	  bmp_cfg_db_set_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), (gint)pp->valueL);
	  break;
	case kBool:
	  bmp_cfg_db_set_bool(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueB);
	  break;
	default:
	  break;
	}
      bmp_cfg_db_close(cfg);
#else
# if __AUDACIOUS_PLUGIN_API__ < 16
      ConfigDb * cfg = aud_cfg_db_open();
# else
      mcs_handle_t * cfg = _aud_api_table->configdb_api->cfg_db_open();
# endif
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  aud_cfg_db_set_float(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueF);
	  break;
	case kLong:
	case kSelect:
	  aud_cfg_db_set_int(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), (gint)pp->valueL);
	  break;
	case kBool:
	  aud_cfg_db_set_bool(cfg, (gchar*)psName, (gchar*)pp->parameterConfigName.c_str(), pp->valueB);
	  break;
	default:
	  break;
	}
      aud_cfg_db_close(cfg);
#endif
#endif
    }
    
    static GtkWidget * createGUILabel(PluginParameter * pp)
    {
      GtkWidget * go = NULL;
      std::string labelString = pp->parameterDisplayName + std::string(" ") + pp->parameterLabel;
      go = gtk_label_new(labelString.c_str());
      return go;
    }
    
    static GtkWidget * createGUIObject(PluginParameter * pp)
    {
      GtkWidget * go = NULL;
      switch(pp->typeOfParameter)
	{
	case kFloat:
	case kLong:
	  pp->gtkAdj = gtk_adjustment_new(pp->minParam, pp->minParam, pp->maxParam+1.0, 0.01, 0.1, 1.0);
	  go = gtk_hscale_new(GTK_ADJUSTMENT(pp->gtkAdj));
	  gtk_widget_set_usize(go, 400, 35);
	  gtk_scale_set_digits(GTK_SCALE(go), (gint)pp->displayDigit);
	  break;
	case kBool:
	  go = gtk_check_button_new();
	  pp->gtkAdj = GTK_OBJECT(go);
	  break;
	case kSelect:
	  // run once
	  if(pp->menuStrings.size() == 0) splitStringToVector(&pp->parameterSelect, &pp->menuStrings, DELIMITER);
	  pp->selectMenu.clear();
	  GtkWidget *menu = gtk_menu_new();
	  GtkOptionMenu *omenu = GTK_OPTION_MENU(gtk_option_menu_new());
	  for(unsigned i = 0;i < pp->menuStrings.size();i ++)
	    {
	      GtkWidget * item = gtk_menu_item_new_with_label(pp->menuStrings[i].c_str());
	      gtk_signal_connect(GTK_OBJECT(item), "activate", GTK_SIGNAL_FUNC(guiCallBack), pp);
	      gtk_widget_show(item);
	      gtk_menu_append(GTK_MENU(menu), item);
	      pp->selectMenu.insert(std::pair<GtkObject*,unsigned>(GTK_OBJECT(item),i));
	    }
	  gtk_option_menu_remove_menu(omenu);
	  gtk_option_menu_set_menu(omenu, menu);
	  go = GTK_WIDGET(omenu);
	  pp->gtkAdj = GTK_OBJECT(go);
	  break;
	}
      return go;
    }
    
    static void setGUIObjectValue(PluginParameter * pp)
    {
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(pp->gtkAdj), pp->valueF);
	  break;
	case kLong:
	  gtk_adjustment_set_value(GTK_ADJUSTMENT(pp->gtkAdj), pp->valueL);
	  break;
	case kBool:
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pp->gtkAdj), pp->valueB);
	  break;
	case kSelect:
	  gtk_option_menu_set_history(GTK_OPTION_MENU(pp->gtkAdj), pp->valueL);
	  break;
	}
    }
    
    static void registerGUICallback(GtkObject * go, PluginParameter * pp)
    {
      switch(pp->typeOfParameter)
	{
	case kFloat:
	case kLong:
	  // GtkAdjustment
	  gtk_signal_connect(GTK_OBJECT(pp->gtkAdj), "value_changed", GTK_SIGNAL_FUNC(guiCallBack), pp);
	  break;
	case kBool:
	  // GtkCheckButton
	  gtk_signal_connect(go, "toggled", GTK_SIGNAL_FUNC(guiCallBack), pp);
	  break;
	case kSelect:
	  // GtkOptionMenu (GTK>2.0)
	  // gtk_signal_connect(go, "changed", GTK_SIGNAL_FUNC(guiCallBack), pp);
	  // we have to use GtkMenuItem's "activate"
	  break;
	}
    }

    // GUI callbacks
    static void guiCallBack(GtkObject * go, gpointer data)
    {
      PluginParameter * pp = (PluginParameter*)data;
      std::map<GtkObject*,unsigned>::iterator result;
      switch(pp->typeOfParameter)
	{
	case kFloat:
	  pp->valueF = GTK_ADJUSTMENT(go)->value;
	  break;
	case kLong:
	  pp->valueL = (long)GTK_ADJUSTMENT(go)->value;
	  break;
	case kBool:
	  pp->valueB = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(go));
	  break;
	case kSelect:
	  result = pp->selectMenu.find(go);
	  if(result != pp->selectMenu.end()) pp->valueL = (long)result->second;
	  break;
	default:
	  break;
	}
      if(pp->isRTParameter == true)
	execCallBack(pp);
      else
	pp->needNRTCall = true;
    }
    
    static void conf_save_cb(GtkButton * b, gpointer d)
    {
      if(d == NULL) return;
      libxmmsplugin * cl = (libxmmsplugin*)d;
      fprintf(stderr, "libxmmsplugin<%s>: save:\n", cl->configSectionString);
      for(unsigned i = 0;i < cl->ParamsV.size();i ++)
	{
	  PluginParameter * pp = &(cl->ParamsV[i]);
	  writeConfig(pp, cl->configSectionString);
	}
    }

    static void conf_close_cb(GtkButton * b, gpointer d)
    {
      if(d == NULL) return;
      libxmmsplugin * cl = (libxmmsplugin*)d;
      fprintf(stderr, "libxmmsplugin<%s>: close:\n", cl->configSectionString);
      gtk_widget_destroy(GTK_WIDGET(cl->conf_dialog));
      cl->conf_dialog = NULL;
    }
    
    static void conf_reload_cb(GtkButton * b, gpointer d)
    {
      if(d == NULL) return;
      libxmmsplugin * cl = (libxmmsplugin*)d;
      fprintf(stderr, "libxmmsplugin<%s>: reload:\n", cl->configSectionString);
      for(unsigned i = 0;i < cl->ParamsV.size();i ++)
	{
	  PluginParameter * pp = &(cl->ParamsV[i]);
	  gboolean result = readConfig(pp, cl->configSectionString);
	  if(result == true)
	    setGUIObjectValue(pp);
	  else
	    setDefault(pp);
	  if(pp->isRTParameter == true)
	    execCallBack(pp);
	  else
	    pp->needNRTCall = true;
	}
    }
    
    static void conf_default_cb(GtkButton * b, gpointer d)
    {
      if(d == NULL) return;
      libxmmsplugin * cl = (libxmmsplugin*)d;
      fprintf(stderr, "libxmmsplugin<%s>: default:\n", cl->configSectionString);
      for(unsigned i = 0;i < cl->ParamsV.size();i ++)
	{
	  PluginParameter * pp = &(cl->ParamsV[i]);
	  setDefault(pp);
	  setGUIObjectValue(pp);
	  if(pp->isRTParameter == true)
	    execCallBack(pp);
	  else
	    pp->needNRTCall = true;
	}
    }
    
    SLOTP origLR, orig, reverb;
    gint plugin_rate, plugin_ch;
    void (*_mod_samples)(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate);
    const char *aboutString, *productString, *configSectionString;
    std::vector<PluginParameter> ParamsV;
    GtkWidget *conf_dialog;
  };
};
