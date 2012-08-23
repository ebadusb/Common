/*****************************************************************************\
**
** VistaRecordUtil.h
**
** Utility classes for extracting vista data.
**
\*****************************************************************************/

/* Includes ******************************************************************/

#include <algorithm>
#include <iostream>
#include <list>
#include <set>

namespace DECODER {
namespace UTIL {

/* Constants *****************************************************************/

const DECODER::Uint16 VISTA_RecordType						= 0x5356;

const DECODER::Uint16 VISTA_AlarmSubType					= 0x0001;
const DECODER::Uint16 VISTA_AlarmResponseSubType		= 0x0002;
const DECODER::Uint16 VISTA_BarcodeScanSubType			= 0x0003;
const DECODER::Uint16 VISTA_DonorRejectedSubType		= 0x0004;
const DECODER::Uint16 VISTA_ParamAdjustSubType			= 0x0005;
const DECODER::Uint16 VISTA_PFRStatusSubType				= 0x0006;
const DECODER::Uint16 VISTA_DonorVitalsSubType			= 0x0007;
const DECODER::Uint16 VISTA_RunSummarySubType_51_Only	= 0x0008;   // Only used in 5.1
const DECODER::Uint16 VISTA_AuditTrailSubType			= 0x0009;
const DECODER::Uint16 VISTA_RunSummarySubType_52_Only	= 0x000A;   // Only used in 5.2
const DECODER::Uint16 VISTA_CatalogListSubType			= 0x000B;
const DECODER::Uint16 VISTA_InterfaceStateSubType		= 0x000C;
const DECODER::Uint16 VISTA_RunSummarySubType			= 0x000D;

const DECODER::Uint16 MAX_SCAN_LENGTH = 120;
const DECODER::Uint16 BARCODE_ID_SIZE = 80 + 1;
const DECODER::Uint16 TRIGGER_DONOR_VITALS_A_CHANGE = 1;
const DECODER::Uint16 TRIGGER_DONOR_VITALS_B_CHANGE = 2;

/* Types *********************************************************************/

union VistaTriggerDataType {
	struct { // Donor Vitals A
		DECODER::Int32 height;  // cm
		DECODER::Int32 weight;  // kg
		DECODER::Int32 gender;  // 0 = MALE, 1 = FEMALE
   } donor_vitals_a;

	struct { // Donor Vitals B
		DECODER::Int32 bloodType;
		DECODER::Int32 plateletCount;
		DECODER::Int32 hematocrit;
   } donor_vitals_b;

	DECODER::Int32 value;
};

struct VistaRecordAlarm {
	DECODER::Uint32 name;
	DECODER::Uint32 state;
};

struct VistaRecordAlarmResponse {
	DECODER::Uint32 name;
	DECODER::Uint32 response;
};

struct VistaRecordBarcodeScan {
	DECODER::Uint32 category;
	DECODER::Uint32 sequence;
	DECODER::Uint32 status;
	DECODER::Uint32 vista_response;
	DECODER::Uint32 scan_len;
	DECODER::Uint32 scan_symbology;
	DECODER::Char   scan_string[MAX_SCAN_LENGTH];
};

struct VistaRecordDonorVitals {
	DECODER::Uint32 bloodType;
	DECODER::Uint32 downloadStatus;
	DECODER::Uint32 gender;
	DECODER::Uint32 infoStatus;
	DECODER::Uint32 vitalsStatus;
	DECODER::Float  hct;
	DECODER::Float  pltPrecount;
	DECODER::Float  procTBV;
	DECODER::Float  safetyTBV;
	DECODER::Float  donorHeight;
	DECODER::Float  donorWeight;
};

struct VistaRecordAuditTrail {
	DECODER::Uint32 sequenceNumber;
   time_t          timestamp;
	DECODER::Uint32 trigger;
   VistaTriggerDataType data;
	DECODER::Uint32 status;
	DECODER::Uint32 device;
	DECODER::Char   badge[BARCODE_ID_SIZE];
};

struct VistaRecordCatalogItem {
	DECODER::Int32 catalogNumber;
	DECODER::Int32 tubingSetCode;
	DECODER::Char  barcode[80];
};

struct VistaRecordInterfaceState {
	DECODER::Uint32 privilegedState;
	DECODER::Uint32 donorDownloaded;
	DECODER::Uint32 pfrStatus;
	DECODER::Uint32 originalSequenceNumber;
};

struct VistaRecordRunSummary {
	DECODER::Uint32 acToDonor;
	DECODER::Float  postDonorHCT;
	DECODER::Uint32 postDonorPltCount;
	DECODER::Uint32 startofAASTime;
	DECODER::Uint32 rinsebackComplete;
	DECODER::Uint32 rbcIndividualReporting;
	DECODER::Uint32 PasOfflineVolume;
	DECODER::Uint32 Ras1OfflineVolume;
	DECODER::Uint32 Ras2OfflineVolume;
	DECODER::Float  RBC1ActualHct;
	DECODER::Float  RBC2ActualHct;
	DECODER::Uint32 RBCResidualInSet;
	DECODER::Uint32 PlasmaResidualInSet;
	DECODER::Uint32 labelPlatelet;
	DECODER::Uint32 labelPlasma;
	DECODER::Uint32 labelRBC1;
	DECODER::Uint32 labelRBC2;
};

typedef std::ostream Stream;

/* Classes *******************************************************************/

class ProcessVistaAlarmRec {
public:
	ProcessVistaAlarmRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_AlarmSubType) {
			const VistaRecordAlarm *rec = reinterpret_cast<const VistaRecordAlarm*>(&record.mData[4]);

			mStream << "VISTA_AlarmSubType" << std::endl
				<< "\t name:  " << rec->name << std::endl
				<< "\t state: " << rec->state << std::endl 
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaAlarmResponseRec {
public:
	ProcessVistaAlarmResponseRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_AlarmResponseSubType) {
			const VistaRecordAlarmResponse *rec = reinterpret_cast<const VistaRecordAlarmResponse*>(&record.mData[4]);

			mStream << "VISTA_AlarmResponseSubType" << std::endl
				<< "\t name:     " << rec->name << std::endl
				<< "\t response: " << rec->response << std::endl 
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaBarcodeScanRec {
public:
	ProcessVistaBarcodeScanRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_BarcodeScanSubType) {
			const VistaRecordBarcodeScan *rec = reinterpret_cast<const VistaRecordBarcodeScan*>(&record.mData[4]);

			mStream << "VISTA_BarcodeScanSubType" << std::endl
				<< "\t category:       " << rec->category << std::endl
				<< "\t sequence:       " << rec->sequence << std::endl 
				<< "\t status:         " << rec->status << std::endl 
				<< "\t vista_response: " << rec->vista_response << std::endl 
				<< "\t scan_len:       " << rec->scan_len << std::endl 
				<< "\t scan_symbology: " << rec->scan_symbology << std::endl 
				<< "\t scan_string:    " << rec->scan_string << std::endl 
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaDonorRejectedRec {
public:
	ProcessVistaDonorRejectedRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_DonorRejectedSubType) {
			mStream << "VISTA_DonorRejectedSubType" << std::endl
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaParamAdjustRec {
public:
	ProcessVistaParamAdjustRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_ParamAdjustSubType) {
			mStream << "VISTA_ParamAdjustSubType" << std::endl
				<< "\t value:" << *reinterpret_cast<const Int32*>(&record.mData[4]) << std::endl
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaPFRStatusRec {
public:
	ProcessVistaPFRStatusRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_PFRStatusSubType) {
			mStream << "VISTA_PFRStatusSubType" << std::endl
				<< "\t value:" << *reinterpret_cast<const Int32*>(&record.mData[4]) << std::endl
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaDonorVitalsRec {
public:
	ProcessVistaDonorVitalsRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_DonorVitalsSubType) {
			const VistaRecordDonorVitals *rec = reinterpret_cast<const VistaRecordDonorVitals*>(&record.mData[4]);

			mStream << "VISTA_DonorVitalsSubType" << std::endl
				<< "\t bloodType:      " << rec->bloodType << std::endl
				<< "\t downloadStatus: " << rec->downloadStatus << std::endl 
				<< "\t gender:         " << rec->gender << std::endl 
				<< "\t infoStatus:     " << rec->infoStatus << std::endl 
				<< "\t vitalsStatus:   " << rec->vitalsStatus << std::endl 
				<< "\t hct:            " << rec->hct << std::endl 
				<< "\t pltPrecount:    " << rec->pltPrecount << std::endl 
				<< "\t procTBV:        " << rec->procTBV << std::endl 
				<< "\t safetyTBV:      " << rec->safetyTBV << std::endl 
				<< "\t donorHeight:    " << rec->donorHeight << std::endl 
				<< "\t donorWeight:    " << rec->donorWeight << std::endl 
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaAuditTrailRec {
public:
	ProcessVistaAuditTrailRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_AuditTrailSubType) {
			const VistaRecordAuditTrail *rec = reinterpret_cast<const VistaRecordAuditTrail*>(&record.mData[4]);

			mStream << "VISTA_AuditTrailSubType" << std::endl
				<< "\t sequenceNumber: " << rec->sequenceNumber << std::endl
				<< "\t timestamp:      " << rec->timestamp << std::endl
				<< "\t trigger:        " << rec->trigger << std::endl;

			if (rec->trigger == TRIGGER_DONOR_VITALS_A_CHANGE) {
				mStream << "\t\t data.donor_vitals_a.height:     " << rec->data.donor_vitals_a.height << std::endl
					<< "\t\t data.donor_vitals_a.weight: " << rec->data.donor_vitals_a.weight << std::endl
					<< "\t\t data.donor_vitals_a.gender:    " << rec->data.donor_vitals_a.gender << std::endl;
			} 
			else if (rec->trigger == TRIGGER_DONOR_VITALS_B_CHANGE) {
				mStream << "\t\t data.donor_vitals_b.bloodType:     " << rec->data.donor_vitals_b.bloodType << std::endl
					<< "\t\t data.donor_vitals_b.plateletCount: " << rec->data.donor_vitals_b.plateletCount << std::endl
					<< "\t\t data.donor_vitals_b.hematocrit:    " << rec->data.donor_vitals_b.hematocrit << std::endl;
			}
			else {
				mStream << "\t\t data.value:                        " << rec->data.value << std::endl;
			}

			mStream << "\t status:         " << rec->status << std::endl
				<< "\t device:         " << rec->device << std::endl
				<< "\t badge:          " << rec->badge << std::endl
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaCatalogItemRec {
public:
	ProcessVistaCatalogItemRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_CatalogListSubType) {
			Int32 count = *reinterpret_cast<const Int32*>(&record.mData[4]);
			Int32 index = sizeof(Uint16) * 2 + sizeof(Int32);
			const VistaRecordCatalogItem *rec = NULL;

			for (Int32 i = 0; i < count; ++i, index += sizeof(VistaRecordCatalogItem)) {
				rec = reinterpret_cast<const VistaRecordCatalogItem*>(&record.mData[index]);

				mStream << "VISTA_CatalogListSubType[" << i << "]" << std::endl
					<< "\t catalogNumber: " << rec->catalogNumber << std::endl
					<< "\t tubingSetCode: " << rec->tubingSetCode << std::endl
					<< "\t barcode:       " << rec->barcode << std::endl
					<< std::endl;
			}
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaInterfaceStateRec {
public:
	ProcessVistaInterfaceStateRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_InterfaceStateSubType) {
			const VistaRecordInterfaceState *rec = reinterpret_cast<const VistaRecordInterfaceState*>(&record.mData[4]);

			mStream << "VISTA_InterfaceStateSubType" << std::endl
				<< "\t privilegedState:        " << rec->privilegedState << std::endl
				<< "\t donorDownloaded:        " << rec->donorDownloaded << std::endl 
				<< "\t pfrStatus:              " << rec->pfrStatus << std::endl 
				<< "\t originalSequenceNumber: " << rec->originalSequenceNumber << std::endl 
				<< std::endl;
		}
	}

private:
	Stream &mStream;
};

class ProcessVistaRunSummaryRec {
public:
	ProcessVistaRunSummaryRec(Stream &stream) : mStream(stream) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == VISTA_RecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == VISTA_RunSummarySubType) {
			const VistaRecordRunSummary *rec = reinterpret_cast<const VistaRecordRunSummary*>(&record.mData[4]);

			mStream << "VISTA_RunSummarySubType" << std::endl
				<< "\t acToDonor:              " << rec->acToDonor << std::endl 
				<< "\t postDonorHCT:           " << rec->postDonorHCT << std::endl 
				<< "\t postDonorPltCount:      " << rec->postDonorPltCount << std::endl 
				<< "\t startofAASTime:         " << rec->startofAASTime << std::endl 
				<< "\t rinsebackComplete:      " << rec->rinsebackComplete << std::endl 
				<< "\t rbcIndividualReporting: " << rec->rbcIndividualReporting << std::endl 
				<< "\t PasOfflineVolume:       " << rec->PasOfflineVolume << std::endl 
				<< "\t Ras1OfflineVolume:      " << rec->Ras1OfflineVolume << std::endl 
				<< "\t Ras2OfflineVolume:      " << rec->Ras2OfflineVolume << std::endl 
				<< "\t RBC1ActualHct:          " << rec->RBC1ActualHct << std::endl 
				<< "\t RBC2ActualHct:          " << rec->RBC2ActualHct << std::endl 
				<< "\t RBCResidualInSet:       " << rec->RBCResidualInSet << std::endl 
				<< "\t PlasmaResidualInSet:    " << rec->PlasmaResidualInSet << std::endl 
				<< "\t labelPlatelet:          " << rec->labelPlatelet << std::endl 
				<< "\t labelPlasma:            " << rec->labelPlasma << std::endl 
				<< "\t labelRBC1:              " << rec->labelRBC1 << std::endl 
				<< "\t labelRBC2:              " << rec->labelRBC2 << std::endl;

			Int32 count = (record.mData.size() - (sizeof(Uint16) * 2) - sizeof(VistaRecordRunSummary)) / sizeof(Uint32);
			Int32 index = sizeof(Uint16) * 2 + sizeof(VistaRecordRunSummary);

			for (Int32 i = 0; i < count; ++i, index += sizeof(Uint32)) {
				mStream << "\t pqiData[";
				mStream.width(3);
				mStream.fill('0');
				mStream << i;
				mStream <<  "]:           ";
				mStream << *reinterpret_cast<const Uint32*>(&record.mData[index]) << std::endl;
			}
		}
	}

private:
	Stream &mStream;
};

} // namespace UTIL
} // namespace DECODER

