/*  Freeverb XMMS plugin
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
#include "libxmmsplugin.hpp"
#include <freeverb/revmodel.hpp>

#define DEFAULTMAXVFS 192000
#define DEFAULTFS 44100

static const char *about_text =
  "Freeverb3 "VERSION"\n"
  "Freeverb (v3F)\n"
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
  "http://freeverb3.sourceforge.net/\n"
  "Original Freeverb\n"
  "Copyright (C) 2000 Jezar at Dreampoint\n";
static const char * productString = "Freeverb3 "VERSION" [Freeverb(v3F)]";
static const char * configSectionString = "freeverb3_plugin_revmodel";

#ifdef PLUGDOUBLE
static fv3::revmodel_ DSP;
typedef fv3::slot_ SLOTP;
#else
static fv3::revmodel_f DSP;
typedef fv3::slot_f SLOTP;
#endif

//static long converter_type = SRC_SINC_FASTEST;
static long converter_type = SRC_ZERO_ORDER_HOLD;

static int currentfs = 0;
static pthread_mutex_t plugin_mutex;
static gboolean plugin_available = false;
static fv3::libxmmsplugin *XMMSPlugin = NULL;

static void _wet(pfloat_t t){DSP.setwet(t);};
static void _dry(pfloat_t t){DSP.setdry(t);};
static void _roomsize(pfloat_t t){DSP.setroomsize(t);};
static void _damp(pfloat_t t){DSP.setdamp(t);};
static void _width(pfloat_t t){DSP.setwidth(t);};
static void _idelay(pfloat_t t){long iDelay = (long)((float)DSP.getCurrentFs()*t/1000.0f);
  DSP.setInitialDelay(iDelay);};
static void _factor(long t){DSP.setOverSamplingFactor(t,converter_type);};

// configurations
static PluginParameterTable ppConfTable[] = {
  {"Dry","dry","", "",                 0.0,1.0,  2,  0.4, 0,true,true,false,kFloat,(void*)_dry,},
  {"Wet","wet","", "",                 0.0,1.0,  2,  0.3, 0,true,true,false,kFloat,(void*)_wet,},
  {"Damp","damp","", "",               0.0,1.0,  2,  0.2, 0,true,true,false,kFloat,(void*)_damp,},
  {"Roomsize","roomsize","", "",       0.0,1.0,  2,  0.7, 0,true,true,false,kFloat,(void*)_roomsize,},
  {"Width","width","", "",             0.0,1.0,  2,  0.9, 0,true,true,false,kFloat,(void*)_width,},
  {"Initial Delay","idelay","", "", -500.0,500.0,0, 50.0, 0,true,false,true,kFloat,(void*)_idelay,},
  {"OSFactor","osfactor","", "",         1,6,    0,   0,  2,true,false,true,kLong, (void*)_factor,},
};

static void
#ifdef __GNUC__
__attribute__ ((constructor))
#endif
plugin_init(void)
{
  std::fprintf(stderr, "reverbm.cpp: plugin_init()\n");
  pthread_mutex_init(&plugin_mutex, NULL);
}

static void
#ifdef __GNUC__
__attribute__ ((destructor))
#endif
plugin_fini(void)
{
  std::fprintf(stderr, "reverbm.cpp: plugin_fini()\n");
  pthread_mutex_destroy(&plugin_mutex);
}

static void mod_samples(pfloat_t * iL, pfloat_t * iR, pfloat_t * oL, pfloat_t * oR, gint length, gint srate)
{
  if(pthread_mutex_trylock(&plugin_mutex) == EBUSY) return;
  if(plugin_available != true||XMMSPlugin == NULL)
    {
      pthread_mutex_unlock(&plugin_mutex);
      return;
    }
  if(currentfs != srate)
    {
      fprintf(stderr, "reverbm.cpp: Fs %d -> %d, resetAll\n", currentfs, srate);
      currentfs = srate;
      DSP.setCurrentFs(srate);
    }
  XMMSPlugin->callNRTParameters();
  DSP.processreplace(iL,iR,oL,oR,length);
  pthread_mutex_unlock(&plugin_mutex);
}

static void init(void)
{
#ifndef __GNUC__
  plugin_init();
#endif
  fprintf(stderr, "reverbm.cpp: init()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = true;
  if(XMMSPlugin != NULL) delete XMMSPlugin;
  XMMSPlugin = new fv3::libxmmsplugin(ppConfTable, sizeof(ppConfTable)/sizeof(PluginParameterTable),
				      about_text, productString, configSectionString);  
  XMMSPlugin->registerModSamples(mod_samples);
  currentfs = 0;
  pthread_mutex_unlock(&plugin_mutex);
}

static void cleanup(void)
{
  fprintf(stderr, "reverbm.cpp: cleanup()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = false;
  fprintf(stderr, "reverbm.cpp: WARNING: cleanup during play is not supported!!\n");
  delete XMMSPlugin;
  XMMSPlugin = NULL;
  pthread_mutex_unlock(&plugin_mutex);
#ifndef __GNUC__
  plugin_fini();
#endif
}

static void configure(void){if(XMMSPlugin != NULL)XMMSPlugin->configure();}
static void about(void){if(XMMSPlugin != NULL)XMMSPlugin->about();}
#if __AUDACIOUS_PLUGIN_API__ >= 13
static gint decoder_to_output_time(gint time){return time;}
static gint output_to_decoder_time(gint time){return time;}
static void dsp_start(gint * channels, gint * rate){if(XMMSPlugin != NULL) XMMSPlugin->start(channels, rate);}
static void dsp_process(gfloat ** data, gint * samples){if(XMMSPlugin != NULL) XMMSPlugin->process(data,samples);}
static void dsp_flush(){if(XMMSPlugin != NULL) XMMSPlugin->flush();}
static void dsp_finish(gfloat ** data, gint * samples){if(XMMSPlugin != NULL) XMMSPlugin->finish(data,samples);}
#else
static void query_format(AFormat * fmt, gint * rate, gint * nch)
{if(XMMSPlugin != NULL)XMMSPlugin->query_format(fmt,rate,nch);}
static int  mod_samples(gpointer * d, gint length, AFormat afmt, gint srate, gint nch)
{if(XMMSPlugin != NULL) return XMMSPlugin->mod_samples(d,length,afmt,srate,nch); return 0;}
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
  dsp_start,
  dsp_process,
  dsp_flush,
  dsp_finish,
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

