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

class VEINS_API MyVeinsNodeApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	void ingestRSUBroadcast(std::string csvStr);

	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	bool inaccurateBoolCheck(bool val, float accuracy = 0.9); //used for simulating errors while evaluating and generating information.
	float scoreCalculator(float old, int trueCount, int count);
	float setSendingAccuracy();
	float setEvaluatingAccuracy();
	void initVehicle(int id, bool dontRequestDump=false); //to create new entry in vehicles map and stats collection maps
	//int currentSubscribedServiceId;
	int tempWsmCount;
	cOutVector mvec;
	int2vehMsgHistoryDynamic vehicles;
	int sent;
	float sendingAccuracy;  // to control behaviour of node.
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
	int logSplitFactor;
	int logSplitLevel;
	int logSplitSmallest;
	std::vector<int> logSplitSizes; //(for convenience/less complexity)
	static int node0id;
	//--FOR STATS--
	int sentCorrect;
	int recMsg;
	int recRprt;
	int sentRprt;
	int sentDumpRequests;
	int sentDumps;
	int receivedResponseDumps;
	int receivedDumpRequests;
	int receivedDumps;
	cOutVector sentVector;
	cOutVector sentCorrectVector;
	cOutVector sentReportsVector;
	cOutVector sentDumpsVector;
	cOutVector sentDumpRequestsVector;
	cOutVector receivedResponseDumpsVector;
	cOutVector receivedDumpsVector;
	cOutVector receivedDumpRequestsVector;
	std::tr1::unordered_map<int, cOutVector*> repScoreVector_MIN;
	std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> repScoreVector;

	//--For Sim type
	bool withoutReportDumpSharing;
};

}
