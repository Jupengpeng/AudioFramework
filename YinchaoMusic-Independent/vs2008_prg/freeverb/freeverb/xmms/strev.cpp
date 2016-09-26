/*  Simple Tank Reverb XMMS plugin
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
#include <freeverb/strev.hpp>
#include <freeverb/slot.hpp>
#include <freeverb/fv3_ch_tool.hpp>
#include <libsamplerate2/samplerate2.h>

static const char *about_text =
  "Freeverb3 "VERSION"\n"
  "Simple Tank Reverb (v3F)\n"
  "XMMS/BMP/Audacious Plugin\n"
#ifdef PLUGDOUBLE
  "Double Precision Version\n"
#else
  "Single Precision Version\n"
#endif
  "Copyright (C) 2006-2010 Teru KAMOGASHIRA\n"
  "http://freeverb3.sourceforge.net/";
static const char * productString = "Freeverb3 "VERSION" [SimpleTankRev(v3F)]";
static const char * configSectionString = "freeverb3_plugin_strev";

#ifdef PLUGDOUBLE
static fv3::strev_ DSP;
typedef fv3::slot_ SLOTP;
#else
static fv3::strev_f DSP;
typedef fv3::slot_f SLOTP;
#endif

static int currentfs = 0;
static pthread_mutex_t plugin_mutex;
static gboolean plugin_available = false;
static fv3::libxmmsplugin *XMMSPlugin = NULL;

static void _srcf(long t){DSP.setOverSamplingFactor(t,SRC_ZERO_ORDER_HOLD);
  fprintf(stderr, "strev.cpp: _srcf: SRCFactor: %ld, %ld\n", t, DSP.getOverSamplingFactor());};
static void _width(pfloat_t t){DSP.setwidth(t);};
static void _dry(pfloat_t t){DSP.setdry(t);};
static void _wet(pfloat_t t){DSP.setwet(t);};
static void _id(pfloat_t t){
  long iDelay = (long)((float)currentfs*t/1000.0f);
  DSP.setInitialDelay(iDelay);};
static void _decay(pfloat_t t){DSP.setroomsize(t);};
static void _idiff1(pfloat_t t){DSP.setidiffusion1(t);};
static void _idiff2(pfloat_t t){DSP.setidiffusion2(t);};
static void _diff1(pfloat_t t){DSP.setdiffusion1(t);};
static void _diff2(pfloat_t t){DSP.setdiffusion2(t);};
static void _idamp(pfloat_t t){DSP.setinputdamp(t);};
static void _damp(pfloat_t t){DSP.setdamp(t);};
static void _odamp(pfloat_t t){DSP.setoutputdamp(t);};
static void _spin(pfloat_t t){DSP.setspin(t);};
static void _spind(pfloat_t t){DSP.setspindiff(t);};
static void _spinl(pfloat_t t){DSP.setspinlimit(t);};
static void _wander(pfloat_t t){DSP.setwander(t);};

// configurations
static PluginParameterTable ppConfTable[] = {
  {"SRCFactor","SRCFactor","X", "",  1,6,     0,   0,   3,true,false,true,kLong, (void*)_srcf,},
  {"Dry","Dry","[dB]", "",        -100,20,   2,  -6,   0,true,true,false,kFloat,(void*)_dry,},
  {"Wet","Wet","[dB]", "",        -100,20,   2, -16,   0,true,true,false,kFloat,(void*)_wet,},
  {"IDelay","IDelay","[ms]", "",  -800,800,  0,  50,   0,true,false,true,kFloat,(void*)_id,},
  {"Width","Width","", "",           0,1,     2,   1,   0,true,true,false,kFloat,(void*)_width,},
  {"RT60","RT60","[s]", "",          0,12,    2, 3.5,   0,true,true,false,kFloat,(void*)_decay,},
  {"IDiffusion1","IDiff1","", "",    0,1,     2, 0.75,  0,true,true,false,kFloat,(void*)_idiff1,},
  {"IDiffusion2","IDiff2","", "",    0,1,     2, 0.625, 0,true,true,false,kFloat,(void*)_idiff2,},
  {"Diffusion1","Diff1","", "",      0,1,     2, 0.7,   0,true,true,false,kFloat,(void*)_diff1,},
  {"Diffusion2","Diff2","", "",      0,1,     2, 0.5,   0,true,true,false,kFloat,(void*)_diff2,},
  {"IDamp","IDamp","[Hz]", "",       0,20000, 2, 16000, 0,true,true,false,kFloat,(void*)_idamp,},
  {"Damp","Damp","[Hz]", "",         0,10000, 2, 4000,  0,true,true,false,kFloat,(void*)_damp,},
  {"ODamp","ODamp","[Hz]", "",       0,20000, 2, 12000, 0,true,true,false,kFloat,(void*)_odamp,},
  {"Spin","Spin","[Hz]", "",         0,10,    2, 0.72,  0,true,true,false,kFloat,(void*)_spin,},
  {"SpinDiff","SpinDiff","[Hz]", "", 0,1,     2, 0.11,  0,true,true,false,kFloat,(void*)_spind,},
  {"SpinLimit","SpinLimit","[Hz]","",0,20,    2, 10,    0,true,true,false,kFloat,(void*)_spinl,},
  {"Wander","Wander","", "",         0,0.9,   2, 0.6,   0,true,true,false,kFloat,(void*)_wander,},
  {"DCCut","DCCut","[Hz]", "",       0,100,   2, 6,     0,true,true,false,kFloat,(void*)_wander,},
};

static void
#ifdef __GNUC__
__attribute__ ((constructor))
#endif
plugin_init(void)
{
  std::fprintf(stderr, "strev.cpp: plugin_init()\n");
  pthread_mutex_init(&plugin_mutex, NULL);
}

static void
#ifdef __GNUC__
__attribute__ ((destructor))
#endif
plugin_fini(void)
{
  std::fprintf(stderr, "strev.cpp: plugin_fini()\n");
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
      fprintf(stderr, "strev.cpp: mod_samples: Fs %d -> %d, resetAll\n", currentfs, srate);
      currentfs = srate;
      DSP.setOverSamplingFactor(DSP.getOverSamplingFactor(),SRC_ZERO_ORDER_HOLD);
      DSP.setCurrentFs(srate);
      fprintf(stderr, "strev.cpp: mod_samples: SRCFactor: %ld\n", DSP.getOverSamplingFactor());
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
  fprintf(stderr, "strev.cpp: init()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = true;
  if(XMMSPlugin != NULL) delete XMMSPlugin;
  XMMSPlugin = new fv3::libxmmsplugin(ppConfTable, sizeof(ppConfTable)/sizeof(PluginParameterTable),
				      about_text, productString, configSectionString);  
  XMMSPlugin->registerModSamples(mod_samples);
  pthread_mutex_unlock(&plugin_mutex);
}

static void cleanup(void)
{
  fprintf(stderr, "strev.cpp: cleanup()\n");
  pthread_mutex_lock(&plugin_mutex);
  plugin_available = false;
  fprintf(stderr, "strev.cpp: WARNING: cleanup during play is not supported!!\n");
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
  fprintf(stderr, "freeverb3.cpp: get_eplugin_info()\n");
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
