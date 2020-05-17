#pragma once

#include "veins/veins.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/requestDumpMsg_m.h"
#include "veins/modules/application/traci/reportDumpMsg_m.h"
#include "veins/modules/application/traci/RSUBroadcast_m.h"
#include "veins/modules/application/traci/auxiliaryClassesAndFunctions.h"

namespace veins {

class VEINS_API MyVeinsNodeStub: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	bool inaccurateBoolCheck(bool val, float accuracy = 0.9);
	float setSendingAccuracy();
	float setEvaluatingAccuracy();
	void initVehicle(int id, bool dontRequestDump=false);
	static intSet collusionTargets;
	bool isColluding;
	std::tr1::unordered_map<int,std::tr1::unordered_map<int,bool>> myReports;
	bool enableStats;
	int lastRSUBroadcastId;
	int sent;
	float sendingAccuracy;
	float evaluatingAccuracy;
	float evaluatableMessages;
	cMessage *sendMsgEvt;
	simtime_t messageInterval;
	simtime_t messageIntervalVarianceLimit;
	simtime_t reportGenTime;
	simtime_t reportGenTimeVarianceLimit;
	simtime_t requestResponseDelay;
	simtime_t requestResponseDelayVarianceLimit;
	simtime_t requestDelay;
	simtime_t requestDelayVarianceLimit;
	static int node0id;
	//--FOR STATS--
	static int sentRprtGlobal;
	static int sentMsgGlobal;
	int sentCorrect;
	int recMsg;
	int recRprt;
	int sentRprt;
	int sentCorrectRprt;
	int sentDumpRequests;
	int sentDumps;
	int receivedResponseDumps;
	int receivedDumpRequests;
	int receivedDumps;
	int receivedRSUBroadcasts;
	static cOutVector* sentRprtGlobalVector;
	static cOutVector* sentMsgGlobalVector;
	cOutVector sentVector;
	cOutVector sentCorrectVector;
	cOutVector sentReportsVector;
	cOutVector sentCorrectReportsVector;
	cOutVector sentDumpsVector;
	cOutVector sentDumpRequestsVector;
	cOutVector receivedResponseDumpsVector;
	cOutVector receivedDumpsVector;
	cOutVector receivedDumpRequestsVector;
	cOutVector receivedRSUBroadcastsVector;
	cOutVector myAccuracyVector;

	//--For Sim type
	bool withoutReportDumpSharing;
};

}
