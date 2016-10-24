class _FV3_(iir_1st)
{
 public:
  _FV3_(iir_1st)();
  virtual _FV3_(~iir_1st)();
  void printconfig();
  void mute();
  _fv3_float_t get_A1(){return 1;}
  _fv3_float_t get_A2(){return a2;}
  _fv3_float_t get_B1(){return b1;}
  _fv3_float_t get_B2(){return b2;}
  void set_A2(_fv3_float_t v){a2=v;}
  void set_B1(_fv3_float_t v){b1=v;}
  void set_B2(_fv3_float_t v){b2=v;}
  
  void setCoefficients(_fv3_float_t _b1, _fv3_float_t _b2, _fv3_float_t _a2);

  void setLPFA(_fv3_float_t fc, _fv3_float_t fs);
  void setHPFA(_fv3_float_t fc, _fv3_float_t fs);
  void setLSFA(_fv3_float_t f1, _fv3_float_t f2, _fv3_float_t fs);
  void setHSFA(_fv3_float_t f1, _fv3_float_t f2, _fv3_float_t fs);
  void setHPFwLFSA(_fv3_float_t fc, _fv3_float_t fs);

  void setLPFB(_fv3_float_t fc, _fv3_float_t fs);
  void setHPFB(_fv3_float_t fc, _fv3_float_t fs);
  void setLPFC(_fv3_float_t fc, _fv3_float_t fs);
  void setHPFC(_fv3_float_t fc, _fv3_float_t fs);

  void setPole(_fv3_float_t v);
  void setZero(_fv3_float_t v);
  void setPoleLPF(_fv3_float_t fc, _fv3_float_t fs);
  void setPoleHPF(_fv3_float_t fc, _fv3_float_t fs);
  void setZeroLPF(_fv3_float_t fc, _fv3_float_t fs);
  void setZeroHPF(_fv3_float_t fc, _fv3_float_t fs);
  
  inline _fv3_float_t process(_fv3_float_t input)
  {
    return this->processd1(input);
  }

  // Direct form I
  inline _fv3_float_t processd1(_fv3_float_t input)
  {
    _fv3_float_t output = input * b1 + y1;
    y1 = output * a2 + input * b2;
    UNDENORMAL(output);
    return output;
  }
  
 private:
  _FV3_(iir_1st)(const _FV3_(iir_1st)& x);
  _FV3_(iir_1st)& operator=(const _FV3_(iir_1st)& x);
  _fv3_float_t a2, b1, b2, y1;
};

class _FV3_(efilter)
{
public:
  _FV3_(efilter)();
  virtual _FV3_(~efilter)();
  inline _fv3_float_t processL(_fv3_float_t input)
  {
    return lpfL.process(hpfL.process(input*(-1)));
  }
  inline _fv3_float_t processR(_fv3_float_t input)
  {
    return lpfR.process(hpfR.process(input*(-1)));
  }
  void setLPF(_fv3_float_t val);
  _fv3_float_t getLPF();
  void setHPF(_fv3_float_t val);
  _fv3_float_t getHPF();
  void mute();
  
private:
  _FV3_(efilter)(const _FV3_(efilter)& x );
  _FV3_(efilter)& operator=(const _FV3_(efilter)& x);
  _FV3_(iir_1st) lpfL, lpfR, hpfL, hpfR;
  _fv3_float_t pole, zero;
};

class _FV3_(dccut)
{
 public:
  _FV3_(dccut)();
  virtual _FV3_(~dccut)();

  inline _fv3_float_t process(_fv3_float_t input)
  {
    return this->processd1(input);
  }

  inline _fv3_float_t processp(_fv3_float_t input)
  {
    // if you want precisely normalized signals,
    // the input signals should be *(1+gain)/2.
    return this->processd1(input*(1.+gain)/2.);
  }

  // Direct form I
  inline _fv3_float_t processd1(_fv3_float_t input)
  {
    _fv3_float_t output = input;
    output -= y1; y1 = input;
    output += gain * y2; y2 = output;
    UNDENORMAL(output);
    return output;
  }
  
  void mute();
  void seta(_fv3_float_t val);
  _fv3_float_t geta();
  void setCutOnFreq(_fv3_float_t fc, _fv3_float_t fs);
  _fv3_float_t getCutOnFreq();
  _fv3_float_t getCutOnFreq(_fv3_float_t fs);

 private:
  _FV3_(dccut)(const _FV3_(dccut)& x);
  _FV3_(dccut)& operator=(const _FV3_(dccut)& x);  
  _fv3_float_t gain, y1, y2;
};

class _FV3_(lfo)
{
 public:
  _FV3_(lfo)();
  virtual _FV3_(~lfo)();

  inline _fv3_float_t processo()
  {
    _fv3_float_t new_re = re*arc_re - im*arc_im;
    _fv3_float_t new_im = re*arc_im + im*arc_re;
    UNDENORMAL(new_re); UNDENORMAL(new_im);
    re = new_re; im = new_im;
    return im;
  }

  inline _fv3_float_t processp()
  {
    _fv3_float_t arc_r = std::sqrt(re*re + im*im);
    UNDENORMAL(arc_r);
    re /= arc_r; im /= arc_r;
    return this->processo();
  }

  inline _fv3_float_t processq()
  {
    if(count++ > count_max)
      {
	count = 0;
	return this->processp();
      }
    else
      return this->processo();
  }
  
  inline _fv3_float_t process()
  {
    return this->processq();
  }

  inline _fv3_float_t process(_fv3_float_t input)
  {
    return this->process()*input;
  }

  void mute();
  void setFreq(_fv3_float_t freq, _fv3_float_t fs);
  void setFreq(_fv3_float_t fc);

 private:
  _FV3_(lfo)(const _FV3_(lfo)& x);
  _FV3_(lfo)& operator=(const _FV3_(lfo)& x);
  _fv3_float_t s_fc, re, im, arc_re, arc_im;
  long count_max, count;
};
