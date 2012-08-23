/*****************************************************************************\
**
** PredictDataUtil.h
**
** Utility classes for extracting predict data from datalog files.
**
\*****************************************************************************/

#ifndef PREDICTDATAUTIL_H
#define PREDICTDATAUTIL_H

/* Includes ******************************************************************/

#include <Windows.h>
#include <string>
#include <sstream>
#include <set>

namespace DECODER {
namespace UTIL {

/* Constants *****************************************************************/

static const char *kTagPredictAdjustment = "Adjustment caps:";
static const char *kTagPredictProcData = "Predict: ProcData";
static const char *kTagPredictProcDataReceived = "Predict: ProcDataReceived";
static const char *kTagPredictDonorStats = "Donor Stats.";
static const char *kTagPredictData = "PredictData:";
/* Types *********************************************************************/

struct Donor {
	int id;
	float height;
	float weight;
	float tbv;
	float precount;
	float hct;
	float bmi;
	int female; /* 1-female 0-male*/
	int blood_type;
};

struct Volumes {
	float vin;
	float vac;
	float vplasma_bag;
	float vplasma_bag_ac;
	float vcollect_bag;
	float vcollect_bag_ac;
	float vrbc_bag;
	float vrbc_bag_ac;
	float dose_rbc_bag;
	float vrep;
	float vsaline_bolus;
	float platelet_yield;
	float time_of_vincv; /* Total Collect volume */
	float vincv;
	float vin_epca_start;

	float rbc_s1;
	float rbc_s2;
	float rbc_prime;

	float rbc_s1_double;
	float rbc_s2_double;
	float rbc_double;
};

struct Run {
	int substate;
	float qch_min;
	float qch_cur;
	int npurges;
	float adjusted_hct;
	int current_recovery;
	float vin_clearline;
	time_t wbc_purge_start_time;
	int wbc_purge_type;
	//int substate_start_time;
	//int midrun_entered;
	int predict_request;
	int pre_pir_ramp;
	float qcmr; //Move to Predict: PredictData. Already done for 6.1
};

struct Time {
	float proctime;
};

struct Cassette  {
	int type;
	bool rbc_detector_calibrated; 
}; 

struct Adjustments {
	float qin;
	float qrp;
	float ir;
	float ratio;
};

struct ProcData {
	Volumes volumes;
	Run run;
	Time time;
	Cassette cassette;
};

struct PredictData
{
	float optimizing_sel_proc;
	float pir_qin_limit;
	float last_csf;
	float accum_ydpur;
	float accum_purges;
	float last_calc_qchmax;
	float pca_entered;
	float pir_lockin;
	float op_adj_qinmax;
	float qcmr;
};

typedef std::map<Uint32, Adjustments> AdjustmentsMap;
typedef std::map<Float, ProcData> ProcDataMap;
typedef std::vector<PredictData> PredictDataArr;
typedef std::map<Uint32, Donor> DonorStatsMap;

/* Classes *******************************************************************/

class ProcessPredictAdjustments {
public:
	ProcessPredictAdjustments(AdjustmentsMap &map) : mAdjustmentsMap(map) {}

	void operator()(const DECODER::RecordStreamOutput &data) 
	{ 	
		if(data.mMessage.find(kTagPredictAdjustment) != std::string::npos){
			Adjustments adjustments = { 0 };

			// Adjustment caps:Qin Qrp IR R: 142.000 302.500 1.200 13.700
			sscanf(data.mMessage.c_str(),
					"Adjustment caps:Qin Qrp IR R: %f %f %f %f", 
					&(adjustments.qin),
					&(adjustments.qrp),
					&(adjustments.ir),
					&(adjustments.ratio));

			mAdjustmentsMap[data.mTimeSeconds] = adjustments;
		}
	}
private:
	AdjustmentsMap &mAdjustmentsMap;
};

class ProcessPredictProcData {
public:
	ProcessPredictProcData(ProcDataMap &map) : mProcDataMap(map) {}

	void operator()(const DECODER::RecordStreamOutput &data) 
	{ 	
		if(data.mMessage.find(kTagPredictProcData) != std::string::npos){
			if(data.mMessage.find(kTagPredictProcDataReceived) != std::string::npos)
				return;

			ProcData procData = { 0 };
			float maxQch = 0;
			float EPCAstart = 0;
			
			// Predict: ProcData: subst 10031 yld 0.00000 Vc 0.00000 - 0.00000 Vp 0.00000 - 0.00000 Vrep 0.00000 Vrbc 0.00000 Dose 0.00000 Vin 0.00000 Vac 18.28890 Qch 0.00000 minQch 0.00000 maxQch 0.00000 time 0.00000 PIRSpill false Purges 0 Vincv 0.00000 VincvTm 0.00000 EPCAstart 0.00000 VrbcS1 0.000 VrbcS2 0.000 VrbcS1-double 0.000 VrbcS2-double 0.000 VrbcPr 0.000 Vrbc-double 0.000 CurrentRecovery -1 VinClearLine 0.000 WBCPurgeStartTime 0
			sscanf(data.mMessage.c_str(), "Predict: ProcData: subst %d yld %f Vc %f - %f Vp %f - %f Vrep %f Vrbc %f Dose %f Vin %f Vac %f Qch %f minQch %f maxQch %f time %f PIRSpill %*s Purges %d Vincv %f VincvTm %f EPCAstart %f VrbcS1 %f VrbcS2 %f VrbcS1-double %f VrbcS2-double %f VrbcPr %f Vrbc-double %f PredictRequest %d PrePirRamp %d CurrentRecovery %d VinClearLine %f WBCPurgeStartTime %lu WBCPurgeType %d PredictRequest %d Qcmr %d", 
					&procData.run.substate, 
					&procData.volumes.platelet_yield, 
					&procData.volumes.vcollect_bag,
					&procData.volumes.vcollect_bag_ac, 
					&procData.volumes.vplasma_bag, 
					&procData.volumes.vplasma_bag_ac,
					&procData.volumes.vrep, 
					&procData.volumes.vrbc_bag, 
					&procData.volumes.dose_rbc_bag,
					&procData.volumes.vin, 
					&procData.volumes.vac, 
					&procData.run.qch_cur, 
					&procData.run.qch_min, 
					&maxQch, 
					&procData.time.proctime, 
					/*PIRSpill-ignored notice the %*s */ 
					&procData.run.npurges,
					&procData.volumes.vincv, 
					&procData.volumes.time_of_vincv, 
					&EPCAstart, 
					&procData.volumes.rbc_s1, 
					&procData.volumes.rbc_s2,
					&procData.volumes.rbc_s1_double, 
					&procData.volumes.rbc_s2_double, 
					&procData.volumes.rbc_prime, 
					&procData.volumes.rbc_double,
					&procData.run.predict_request,
					&procData.run.pre_pir_ramp,
					&procData.run.current_recovery, //remove 
					&procData.run.vin_clearline,  //remove
					&procData.run.wbc_purge_start_time, 
					&procData.run.wbc_purge_type,
					&procData.run.predict_request,
					&procData.run.qcmr
					);

			//mProcDataMap[data.mTimeSeconds] = procData;
			mProcDataMap[procData.time.proctime] = procData;
		}
	}
private:
	ProcDataMap &mProcDataMap;
};

class ProcessPredictDonorStats {
public:
	ProcessPredictDonorStats(DonorStatsMap &map) : mDonorStatsMap(map) {}

	void operator()(const DECODER::RecordStreamOutput &data) 
	{ 	
		if(data.mMessage.find(kTagPredictDonorStats) != std::string::npos){
			Donor donor = { 0 };
			float sampleVol = 0;

			// Donor Stats.  TBV: 5498.99600 PreCnt: 198 Hct: 0.40000 Wt: 200.00000 BT: 4 SampleVol: 20.00000
			sscanf(data.mMessage.c_str(),
					"Donor Stats.  TBV: %f PreCnt: %f Hct: %f Wt: %f BT: %d SampleVol: %f",
					&donor.tbv, 
					&donor.precount, 
					&donor.hct, 
					&donor.weight, 
					&donor.blood_type, 
					&sampleVol);

			mDonorStatsMap[data.mTimeSeconds] = donor;
		}
	}
private:
	DonorStatsMap &mDonorStatsMap;
};
class ProcessPredictData {
public:
	ProcessPredictData(PredictDataArr &arr) : mPredictDataArr(arr) {}

	void operator()(const DECODER::RecordStreamOutput &data) 
	{ 	
		if(data.mMessage.find(kTagPredictData) != std::string::npos){
			PredictData predictData = { 0 };
			float maxQch = 0;
			float EPCAstart = 0;
			
			// PredictData: AccumPurges 0.000 AccumYdPur 0.000 LastCalcQchmax 3.000 LastCsf 0.934 OptimizingSelProc 1.000 
			//	PCAEntered 0.000 PIRLockin 0.000 QinMidrun 65.366 OpAdjQinmax 0.000 Qcmr 0.000
			sscanf(data.mMessage.c_str(), "PredictData: AccumPurges %f AccumYdPur %f LastCalcQchmax %f LastCsf %f OptimizingSelProc %f PCAEntered %f PIRLockin %f PirQinLimit %f OpAdjQinmax %f Qcmr %f", 
					&predictData.accum_purges, 
					&predictData.accum_ydpur, 
					&predictData.last_calc_qchmax,
					&predictData.last_csf,
					&predictData.optimizing_sel_proc,
					&predictData.pca_entered,
					&predictData.pir_lockin,
					&predictData.pir_qin_limit,
					&predictData.op_adj_qinmax,
					&predictData.qcmr);
			
			mPredictDataArr.push_back(predictData);
		}
	}
private:
	PredictDataArr &mPredictDataArr;
};
} // namespace UTIL
} // namespace TRACE

#endif // PREDICTDATAUTIL_H

