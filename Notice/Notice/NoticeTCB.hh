#ifndef TCB_H
#define TCB_H

#define TM_COUNT (0x1)
#define TM_WIDTH (0x2)
#define TM_PEAKSUM (0x4)
#define TM_ORPEAKSUM (0x8)

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define TCB_VENDOR_ID (0x0547)
#define TCB_PRODUCT_ID (0x1501)

extern int TCBopen(int sid, libusb_context *ctx);
extern void TCBclose(int sid);
extern void TCBwrite_LT(int sid, unsigned long mid, char *data, int len);
extern void TCBreset(int sid);
extern void TCBresetTIMER(int sid);
extern void TCBstart(int sid);
extern void TCBstop(int sid);
extern unsigned long TCBread_RUN(int sid, unsigned long mid);
extern void TCBwrite_CW(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_CW(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_RL(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_RL(int sid, unsigned long mid);
extern void TCBwrite_GW(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_GW(int sid, unsigned long mid);
extern void TCBwrite_DRAMON(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_DRAMON(int sid, unsigned long mid);
extern void TCBwrite_DACOFF(int sid, unsigned long mid, unsigned long ch,
                            unsigned long data);
extern unsigned long TCBread_DACOFF(int sid, unsigned long mid,
                                    unsigned long ch);
extern void TCBmeasure_PED(int sid, unsigned long mid, unsigned long ch);
extern unsigned long TCBread_PED(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_DLY(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_DLY(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_AMOREDLY(int sid, unsigned long mid, unsigned long ch,
                              unsigned long data);
extern unsigned long TCBread_AMOREDLY(int sid, unsigned long mid,
                                      unsigned long ch);
extern void TCBwrite_THR(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_THR(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_POL(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_POL(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_PSW(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_PSW(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_AMODE(int sid, unsigned long mid, unsigned long ch,
                           unsigned long data);
extern unsigned long TCBread_AMODE(int sid, unsigned long mid,
                                   unsigned long ch);
extern void TCBwrite_PCT(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_PCT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_PCI(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_PCI(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_PWT(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_PWT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_DT(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_DT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_PSS(int sid, unsigned long mid, unsigned long ch,
                         unsigned long data);
extern unsigned long TCBread_PSS(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_RT(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_RT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_SR(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_SR(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_DACGAIN(int sid, unsigned long mid, unsigned long ch,
                             unsigned long data);
extern unsigned long TCBread_DACGAIN(int sid, unsigned long mid,
                                     unsigned long ch);
extern void TCBwrite_TM(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_TM(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_TLT(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_TLT(int sid, unsigned long mid);
extern void TCBwrite_STLT(int sid, unsigned long mid, unsigned long ch,
                          unsigned long data);
extern unsigned long TCBread_STLT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_ZEROSUP(int sid, unsigned long mid, unsigned long ch,
                             unsigned long data);
extern unsigned long TCBread_ZEROSUP(int sid, unsigned long mid,
                                     unsigned long ch);
extern void TCBsend_ADCRST(int sid, unsigned long mid);
extern void TCBsend_ADCCAL(int sid, unsigned long mid);
extern void TCBwrite_ADCDLY(int sid, unsigned long mid, unsigned long ch,
                            unsigned long data);
extern void TCBwrite_ADCALIGN(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_ADCSTAT(int sid, unsigned long mid);
extern void TCBwrite_BITSLIP(int sid, unsigned long mid, unsigned long ch,
                             unsigned long data);
extern void TCBwrite_FMUX(int sid, unsigned long mid, unsigned long ch);
extern unsigned long TCBread_FMUX(int sid, unsigned long mid);
extern void TCBarm_FADC(int sid, unsigned long mid);
extern unsigned long TCBread_FREADY(int sid, unsigned long mid);
extern void TCBwrite_ZSFD(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_ZSFD(int sid, unsigned long mid);
extern void TCBwrite_DSR(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_DSR(int sid, unsigned long mid);
extern void TCBwrite_ST(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_ST(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_PT(int sid, unsigned long mid, unsigned long ch,
                        unsigned long data);
extern unsigned long TCBread_PT(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_DRAMDLY(int sid, unsigned long mid, unsigned long ch,
                             unsigned long data);
extern void TCBwrite_DRAMBITSLIP(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_DRAMTEST(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_DRAMTEST(int sid, unsigned long mid,
                                      unsigned long ch);
extern void TCBwrite_DAQMODE(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_DAQMODE(int sid, unsigned long mid);
extern void TCBwrite_HV(int sid, unsigned long mid, unsigned long ch,
                        float data);
extern float TCBread_HV(int sid, unsigned long mid, unsigned long ch);
extern float TCBread_TEMP(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_ADCMUX(int sid, unsigned long mid, unsigned long ch,
                            unsigned long data);
extern void TCBread_FADCBUF(int sid, unsigned long mid, unsigned long *data);
extern void TCB_ADCALIGN_500(int sid, unsigned long mid);
extern void TCB_ADCALIGN_64(int sid, unsigned long mid);
extern void TCB_ADCALIGN_DRAM(int sid, unsigned long mid);
extern void TCBwrite_RUNNO(int sid, unsigned long data);
extern unsigned long TCBread_RUNNO(int sid);
extern void TCBwrite_GATEDLY(int sid, unsigned long data);
extern unsigned long TCBread_GATEDLY(int sid);
extern void TCBsend_TRIG(int sid);
extern void TCBread_LNSTAT(int sid, unsigned long *data);
extern void TCBread_MIDS(int sid, unsigned long *data);
extern void TCBwrite_PTRIG(int sid, unsigned long data);
extern unsigned long TCBread_PTRIG(int sid);
extern void TCBwrite_TRIGENABLE(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_TRIGENABLE(int sid, unsigned long mid);
extern void TCBwrite_MTHR_NKFADC500(int sid, unsigned long data);
extern unsigned long TCBread_MTHR_NKFADC500(int sid);
extern void TCBwrite_PSCALE_NKFADC500(int sid, unsigned long data);
extern unsigned long TCBread_PSCALE_NKFADC500(int sid);
extern void TCBwrite_MTHR_M64ADC_MUON(int sid, unsigned long data);
extern unsigned long TCBread_MTHR_M64ADC_MUON(int sid);
extern void TCBwrite_PSCALE_M64ADC_MUON(int sid, unsigned long data);
extern unsigned long TCBread_PSCALE_M64ADC_MUON(int sid);
extern void TCBwrite_MTHR_M64ADC_LS(int sid, unsigned long data);
extern unsigned long TCBread_MTHR_M64ADC_LS(int sid);
extern void TCBwrite_PSCALE_M64ADC_LS(int sid, unsigned long data);
extern unsigned long TCBread_PSCALE_M64ADC_LS(int sid);
extern void TCBwrite_MTHR_MUONDAQ(int sid, unsigned long data);
extern unsigned long TCBread_MTHR_MUONDAQ(int sid);
extern void TCBwrite_PSCALE_MUONDAQ(int sid, unsigned long data);
extern unsigned long TCBread_PSCALE_MUONDAQ(int sid);
extern void TCBwrite_EXTOUT(int sid, unsigned long data);
extern unsigned long TCBread_EXTOUT(int sid);
extern void TCBwrite_GATEWIDTH(int sid, unsigned long data);
extern unsigned long TCBread_GATEWIDTH(int sid);
extern void TCBwrite_EXTOUTWIDTH(int sid, unsigned long data);
extern unsigned long TCBread_EXTOUTWIDTH(int sid);
extern unsigned long TCBread_BCOUNT(int sid, unsigned long mid);
extern int TCBread_DATA(int sid, unsigned long mid, unsigned long bcount,
                        unsigned char *data);
extern void TCB_ADCALIGN_125(int sid, unsigned long mid);
extern void TCB_ADCALIGN_MUONDAQ(int sid, unsigned long mid);
extern void TCBwrite_GAIN(int sid, unsigned long mid, unsigned long ch,
                          unsigned long data);
extern unsigned long TCBread_GAIN(int sid, unsigned long mid, unsigned long ch);
extern void TCBwrite_MTHR(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_MTHR(int sid, unsigned long mid);
extern void TCBwrite_PSD_DLY(int sid, unsigned long mid, unsigned long data);
extern unsigned long TCBread_PSD_DLY(int sid, unsigned long mid);
extern void TCBwrite_PSD_THR(int sid, unsigned long mid, float data);
extern float TCBread_PSD_THR(int sid, unsigned long mid);
extern unsigned long TCBread_ADCSTAT_WORD(int sid, unsigned long mid);

extern void TCBwrite_TRIG_SWITCH_NKFADC500(int sid, unsigned long fadc,
                                           unsigned long sadc_muon,
                                           unsigned long sadc_ls,
                                           unsigned long MUONDAQ);
extern unsigned long TCBread_SWITCH_NKFADC500(int sid);
extern void TCBwrite_TRIG_SWITCH_M64ADC_MUON(int sid, unsigned long fadc,
                                             unsigned long sadc_muon,
                                             unsigned long sadc_ls,
                                             unsigned long MUONDAQ);
extern unsigned long TCBread_SWITCH_M64ADC_MUON(int sid);
extern void TCBwrite_TRIG_SWITCH_M64ADC_LS(int sid, unsigned long fadc,
                                           unsigned long sadc_muon,
                                           unsigned long sadc_ls,
                                           unsigned long MUONDAQ);
extern unsigned long TCBread_SWITCH_M64ADC_LS(int sid);
extern void TCBwrite_TRIG_SWITCH_MUONDAQ(int sid, unsigned long fadc,
                                         unsigned long sadc_muon,
                                         unsigned long sadc_ls,
                                         unsigned long MUONDAQ);
extern unsigned long TCBread_SWITCH_MUONDAQ(int sid);

extern unsigned long TCBread_DBG(int sid);

#ifdef __cplusplus
}
#endif

#endif
