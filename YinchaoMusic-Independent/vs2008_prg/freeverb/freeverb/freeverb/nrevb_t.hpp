class _FV3_(nrevb) : public _FV3_(nrev)
{
 public:
  _FV3_(nrevb)() throw(std::bad_alloc);
  virtual _FV3_(~nrevb)();
  virtual void mute();
  
  void setapfeedback(_fv3_float_t value){apfeedback = value;}
  _fv3_float_t getapfeedback(){return apfeedback;}
  
 protected:
  virtual void processloop2(long count, _fv3_float_t *inputL, _fv3_float_t *inputR, _fv3_float_t *outputL, _fv3_float_t *outputR);
  virtual void processloop4(long count, _fv3_float_t *inputL, _fv3_float_t *inputR, _fv3_float_t *outputL, _fv3_float_t *outputR,
			    _fv3_float_t *outRearL, _fv3_float_t *outRearR);
  _fv3_float_t apfeedback;
  // work values
  _fv3_float_t dinput;
    
 private:
  _FV3_(nrevb)(const _FV3_(nrevb)& x);
  _FV3_(nrevb)& operator=(const _FV3_(nrevb)& x);
};
