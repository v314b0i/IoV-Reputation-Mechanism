#pragma once

#include "veins/veins.h"
#include <tr1/unordered_map>
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/myClasses.h"
#include "veins/modules/application/traci/myMiscFunctions.h"

namespace veins {

class VEINS_API MyVeinsRSUApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	//void onWSA(DemoServiceAdvertisment *wsa) override;

	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	void initVehicle(int vehId);
	void stageShift();
	void deleteBasket(reportsBasket *basket);
	int_2_float genarateSecondaryScores(reportsBasket *basket);
	void populateBlacklistedReportersList(int_2_float secondaryScores, std::tr1::unordered_set<int> &blacklist);
	int_2_float genaratePrimaryScores_ReportsBased(reportsBasket *basket, int_2_float secondaryScores,
			std::tr1::unordered_set<int> blacklist);
	int_2_float genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket,
			int_2_float secondaryScores, std::tr1::unordered_set<int> blacklist);
	int_2_float genaratePrimaryScores_MessagesBased(reportsBasket *basket, int_2_float secondaryScores,
			std::tr1::unordered_set<int> blacklist);

	cMessage *stageShiftEvt;
	reportsBasket *currentBasket;
	reportsBasket *stagedBasket;
	int_2_intSet archivedScope; //  { senderId : { msgId : val } }

	simtime_t stageShiftInterval;
};

}
