#pragma once

#include "veins/veins.h"
#include <tr1/unordered_map>
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/myClasses.h"

namespace veins {

class VEINS_API MyVeinsApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
	enum MyApplMessageKinds {
		SEND_INFOMSG_EVT,
		//SEND_REPORTMSG_EVT,
		INFO_MSG,
		REPORT_MSG
	};
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	//void onWSA(DemoServiceAdvertisment *wsa) override;

	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	bool inaccurateBoolCheck(bool val, float accuracy = 0.9); //used for simulating errors while evaluating and generating information.
	float scoreCalculator(float old, int trueCount, int count);
	float setSendingAccuracy();
	void initVehicle(int id, bool dontRequestDump=false); //to create new entry in vehicles map and stats collection maps
	//int currentSubscribedServiceId;
	int2vehStats vehicles;
	int sent;
	float sendingAccuracy;  // to control behaviour of node.
	float evaluatingAccuracy;
	cMessage *sendMsgEvt;
	simtime_t messageInterval;
	simtime_t messageIntervalVarianceLimit;
	simtime_t reportGenTime;
	simtime_t reportGenTimeVarianceLimit;
	simtime_t requestResponseDelay;
	simtime_t requestResponseDelayVarianceLimit;
	simtime_t requestDelay;
	simtime_t requestDelayVarianceLimit;
	//--FOR STATS--
	int sentCorrect;
	int recMsg;
	int recRprt;
	int sentRprt;
	int sentDumpRequests;
	int sentDumps;
	int receivedDumpRequests;
	int receivedDumps;
	cOutVector sentVector;
	cOutVector sentCorrectVector;
	cOutVector sentReportsVector;
	cOutVector sentDumpsVector;
	cOutVector sentDumpRequestsVector;
	cOutVector receivedDumpsVector;
	cOutVector receivedDumpRequestsVector;
	std::tr1::unordered_map<int, cOutVector*> repScoreVector;
	std::tr1::unordered_map<int, cHistogram*> repScoreStats;
	std::tr1::unordered_map<int, cOutVector*> reportedVector;
	std::tr1::unordered_map<int, cOutVector*> reportedTrueVector;
	std::tr1::unordered_map<int, cOutVector*> reportComparisonVector;
	std::tr1::unordered_map<int, cOutVector*> msgVector;
	//--For Sim type
	bool withoutReportDumpSharing;
};

}
