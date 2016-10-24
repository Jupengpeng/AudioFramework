class _FV3_(strev) : public _FV3_(revbase)
{
public:
  _FV3_(strev)() throw(std::bad_alloc);
  virtual _FV3_(~strev)();

  virtual void mute();
  virtual void processreplace(_fv3_float_t *inputL, _fv3_float_t *inputR, _fv3_float_t *outputL, _fv3_float_t *outputR, long numsamples)
    throw(std::bad_alloc);
  
  /**
   * set the RT60 of reverb.
   * @param[in] RT60 in seconds.
   */
  void setroomsize(_fv3_float_t value);
  _fv3_float_t getroomsize();
  
  /**
   * set the cut on frequency of the input signals' DC cut filter.
   * @param[in] value actual freqneucy.
   */
  void setdccutfreq(_fv3_float_t value);
  _fv3_float_t getdccutfreq();
  
  void setidiffusion1(_fv3_float_t value);
  void setidiffusion2(_fv3_float_t value);
  _fv3_float_t getidiffusion1();
  _fv3_float_t getidiffusion2();

  void setdiffusion1(_fv3_float_t value);
  void setdiffusion2(_fv3_float_t value);
  _fv3_float_t getdiffusion1();
  _fv3_float_t getdiffusion2();

  void setinputdamp(_fv3_float_t value);
  _fv3_float_t getinputdamp();

  void setdamp(_fv3_float_t value);
  _fv3_float_t getdamp();

  void setoutputdamp(_fv3_float_t value);
  _fv3_float_t getoutputdamp();

  /**
   * set the modulation frequency of the LFO of the modulated allpass filter.
   * @param[in] value LFO modulation freqneucy.
   */
  void setspin(_fv3_float_t value);
  _fv3_float_t getspin();

  /**
   * set the diff of modulation frequency between L and R.
   * @param[in] value LFO modulation freqneucy diff.
   */
  void setspindiff(_fv3_float_t value);
  _fv3_float_t getspindiff();

  /**
   * set the modulation frequency limiter's frequency.
   * @param[in] value max LFO modulation freqneucy.
   */
  void setspinlimit(_fv3_float_t value);
  _fv3_float_t getspinlimit();

  /**
   * set the modulation strength of the modulated allpass filter.
   * the max modulation duration is 32/29761*1000[ms]
   * @param[in] value strength of the LFO.
   */
  void setwander(_fv3_float_t value);
  _fv3_float_t getwander();

 protected:
  _FV3_(strev)(const _FV3_(strev)& x);
  _FV3_(strev)& operator=(const _FV3_(strev)& x);
  virtual void setFsFactors(long fs, long factor);
  _fv3_float_t rt60, decay, dccutfq, inputdamp, damp, outputdamp;
  _fv3_float_t diff1, diff2, idiff1, idiff2, spin, spindiff, spinlimit, wander;
  _FV3_(allpass) allpassC[FV3_STREV_NUM_ALLPASS_4], allpassC_31_33, allpassC_55_59;
  _FV3_(allpassm) allpassM_23_24, allpassM_46_48;
  _FV3_(delay) delayC_30, delayC_39, delayC_54, delayC_63;
  _FV3_(dccut) dccut1;
  _FV3_(iir_1st) lpf_in, lpfC_30, lpfC_54, lfo1_lpf, lfo2_lpf, out1_lpf, out2_lpf;
  _FV3_(lfo) lfo1, lfo2;
  const static long allpCo[FV3_STREV_NUM_ALLPASS], delayCo[FV3_STREV_NUM_DELAY], idxLCo[FV3_STREV_NUM_INDEX], idxRCo[FV3_STREV_NUM_INDEX];
  const static long allpM_EXCURSION;
  long iLC[FV3_STREV_NUM_INDEX], iRC[FV3_STREV_NUM_INDEX], tankDelay;
};
