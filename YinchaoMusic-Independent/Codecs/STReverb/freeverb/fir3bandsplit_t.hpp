class _FV3_(fir3bandsplit)
{
 public:
  _FV3_(fir3bandsplit)();
  virtual _FV3_(~fir3bandsplit)();
  void init() /*throw(std::bad_alloc)*/;
  
  void setCurrentFs(long fs)
    /*throw(std::bad_alloc)*/;
  long getCurrentFs();

  void setWindow(long w)
    /*throw(std::bad_alloc)*/;
  long getWindow();

  void setLowFreqDivider(_fv3_float_t fc)
    /*throw(std::bad_alloc)*/;
  _fv3_float_t getLowFreqDivider();

  void setHighFreqDivider(_fv3_float_t fc)
    /*throw(std::bad_alloc)*/;
  _fv3_float_t getHighFreqDivider();

  void setTransitionBand(_fv3_float_t fc)
    /*throw(std::bad_alloc)*/;
  _fv3_float_t getTransitionBand();

  void setParameter(_fv3_float_t p)
    /*throw(std::bad_alloc)*/;
  _fv3_float_t getParameter();

  void setFragmentSize(long size)
    /*throw(std::bad_alloc)*/;
  long getFragmentSize();

  void setFactor(long factor)
    /*throw(std::bad_alloc)*/;
  long getFactor();

  void setIR(unsigned o)
    /*throw(std::bad_alloc)*/;

  void splitR(_fv3_float_t *inputL, _fv3_float_t *inputR,
	      _fv3_float_t *outputL1, _fv3_float_t *outputR1,
	      _fv3_float_t *outputL2, _fv3_float_t *outputR2,
	      _fv3_float_t *outputL3, _fv3_float_t *outputR3,
	      long numsamples);
  void mergeR(_fv3_float_t *inputL1, _fv3_float_t *inputR1,
	      _fv3_float_t *inputL2, _fv3_float_t *inputR2,
	      _fv3_float_t *inputL3, _fv3_float_t *inputR3,
	      _fv3_float_t *outputL, _fv3_float_t *outputR,
	      _fv3_float_t f1, _fv3_float_t f2, _fv3_float_t f3,
	      long numsamples);

  void mute();
  long getLatency();
  long getGroupDelay();
  long getFilterLength();
  
 private:
  _FV3_(fir3bandsplit)(const _FV3_(fir3bandsplit)& x);
  _FV3_(fir3bandsplit)& operator=(const _FV3_(fir3bandsplit)& x);
  
  void update() /*throw(std::bad_alloc)*/;
  void allocFilter(long length) /*throw(std::bad_alloc)*/;
  void freeFilter();
  void allocIR(unsigned o) /*throw(std::bad_alloc)*/;
  void freeIR();
  long predictN(long w, _fv3_float_t fc);
  long currentfs;
  long fragmentSize, factor;
  long WINDOW;
  long filterLength;
  _fv3_float_t fcLow, fcHigh, trBand, param;
  _fv3_float_t *lpfF, *bpfF, *hpfF;
  _FV3_(irbase) *lpfM, *hpfM; // bpfM
  _FV3_(delay) groupDelayL, groupDelayR;
};
