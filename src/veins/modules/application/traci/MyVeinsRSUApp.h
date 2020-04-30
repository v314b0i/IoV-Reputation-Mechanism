#pragma once

#include "veins/veins.h"
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/reportDumpMsg_m.h"
#include "veins/modules/application/traci/RSUBroadcast_m.h"
#include "veins/modules/application/traci/auxiliaryClassesAndFunctions.h"
#include <mutex>

namespace veins {

class VEINS_API MyVeinsRSUApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
protected:
	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	void onWSM(BaseFrame1609_4 *wsm) override;
	void ingestReport(int reporterId, int reporteeId, int msgId, bool foundValid);
	void addVehicletoCurrentBasket(int vehId);
	void initVehicleStats(int vehId);
	void stageShift();
	void deleteBasket(reportsBasket *basket);
	int_2_float genarateSecondaryScores(reportsBasket *basket);
	intSet generateBlacklistedReportersList(int_2_float secondaryScores);
	int_2_float genaratePrimaryScores_ReportsBased(reportsBasket *basket, intSet blacklist, std::tr1::unordered_map<int, std::pair<int, int>> &logs);
	int_2_int2float genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket, intSet blacklist, int2VehMsgHistory &logs);
	int_2_int2float genaratePrimaryScores_MessagesBased(reportsBasket *basket, intSet blacklist, int2VehMsgHistory &logs);
	void BroadcastResults(int_2_int2float scores, intSet blacklist);
	cMessage *stageShiftEvt;
	//Static Variables (Common for all RSUs, workaround for inter-RSU communication.....not used for the vehicles of-course.)
	static std::mutex staticMemberAccessLock;
	static reportsBasket *currentBasket;
	static reportsBasket *stagedBasket;
	static int_2_intSet archivedScope; //  { senderId : { msgId : val } }
	int2VehMsgHistory logs;
	int2VehMsgHistory logs_MA;
	int2VehMsgHistory logs_WOBL;
	int2VehMsgHistory logs_MA_WOBL;
	std::tr1::unordered_map<int, std::pair<int, int>> logs_RB;
	bool isMaster;

	simtime_t stageShiftInterval;
	int logSplitFactor;
	int logSplitLevel;
	int logSplitSmallest;
	std::vector<int> logSplitSizes; //(for convenience/less complexity)


	static int node0id; // for naming vehicles correctly.
	//FOR STATS
	static int stageCounter;
	static int recievedReports;
	cOutVector recievedReportsVector;
	static intSet statsInitialisedVehicles;
	static intSet partialStatsInitialisedVehicles;
	std::tr1::unordered_map<int, cOutVector*> secondaryScoreVector;
	std::tr1::unordered_map<int, cOutVector*> primaryScore_ReportsBasedVector;
	std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> primaryScore_MessagesBased_MajorityAbsolutistVector;
	std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> primaryScore_MessagesBased_MajorityAbsolutistVector_WOBL;
	std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> primaryScore_MessagesBasedVector;
	std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> primaryScore_MessagesBasedVector_WOBL;
	void recordPrimaryScores(int2VehMsgHistory &lgs,
			std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> scoresVector);
	std::tr1::unordered_map<int, cOutVector*> primaryScore_RAW_Vector;
	cOutVector secondaryScoreThreshhold;

	cOutVector vehiclesInArchivedScope;
	cOutVector messagesInArchivedScope;

	cOutVector vehiclesInStagedScope;
	cOutVector messagesInStagedScope;
	cOutVector reportsInStagedBasket;
	cOutVector reportersInStagedBasket;
	cOutVector blacklistedReportersInStagedBasket;

	cOutVector vehiclesInCurrentScope;
	cOutVector messagesInCurrentScope;
	cOutVector reportsInCurrentBasket;
	cOutVector reportersInCurrentBasket;

	static int_2_int reportsOn;
	static int_2_int reportsFrom;

	cOutVector testv;
};

}
