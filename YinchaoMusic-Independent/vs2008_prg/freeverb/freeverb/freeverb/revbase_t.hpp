class _FV3_(revbase)
{
public:
  _FV3_(revbase)() throw(std::bad_alloc);
  virtual _FV3_(~revbase)();

  virtual void setCurrentFs(long fs) throw(std::bad_alloc);
  virtual long getCurrentFs();
  virtual void setOverSamplingFactor(long factor, long converter_type);
  virtual void setOverSamplingFactor(long factor);
  virtual long getOverSamplingFactor();
  /**
   * set the delay length of the reverb front sound.
   * @param[in] value reverb time in samples (not seconds).
   * @attention the reverb sound comes first if the value < 0.
   */
  virtual void setInitialDelay(long numsamples);
  virtual long getInitialDelay();
  virtual void         setPreDelay(_fv3_float_t value_ms);
  virtual _fv3_float_t getPreDelay();
  virtual long getLatency();
  virtual void mute();
  virtual void processreplace(_fv3_float_t *inputL, _fv3_float_t *inputR, _fv3_float_t *outputL, _fv3_float_t *outputR, long numsamples)
    throw(std::bad_alloc) = 0;
  /**
   * set the reverb front sound level.
   * @param[in] value dB level.
   */
  virtual void         setwet(_fv3_float_t value);
  virtual _fv3_float_t getwet();
  /**
   * set the dry signal level.
   * @param[in] value dB level.
   */
  virtual void         setdry(_fv3_float_t value);
  virtual _fv3_float_t getdry();
  /**
   * set the width signal level.
   * @param[in] value width level. must be 0~1.
   */
  virtual void         setwidth(_fv3_float_t value);
  virtual _fv3_float_t getwidth();
  virtual void setPrimeMode(bool value);
  virtual bool getPrimeMode();
  virtual void printconfig();

 protected:
  long currentfs, initialDelay;
  _FV3_(delay) delayL, delayR, delayWL, delayWR;
  _fv3_float_t wetDB, wet_scale, wet, wet1, wet2, dryDB, dry_scale, dry, width;
  _FV3_(src) SRC;
  _FV3_(slot) over, overO;
  virtual void growWave(long size) throw(std::bad_alloc);
  virtual void freeWave();
  virtual void update_wet();
  virtual void setFsFactors(long fs, long factor) = 0;
  virtual long f_(long def, _fv3_float_t factor);
  virtual long p_(long def, _fv3_float_t factor);
  bool primeMode;

 private:
  _FV3_(revbase)(const _FV3_(revbase)& x);
  _FV3_(revbase)& operator=(const _FV3_(revbase)& x);  
};
