//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/MyVeinsRSUApp.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/requestDumpMsg_m.h"
#include "veins/modules/application/traci/reportDumpMsg_m.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
using namespace veins;
//TODO stage and archive messages by shifting them from one basket to another instead of shifting whole baskets if approach changes to data centric.
Define_Module(veins::MyVeinsRSUApp);

void MyVeinsRSUApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		currentBasket = new reportsBasket();
		stagedBasket = new reportsBasket();
		stageShiftEvt = new cMessage("Stage Shift Event", STAGE_SHIFT_EVT);
		stageShiftInterval = par("stageShiftInterval");
	} else if (stage == 1) {
		scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
	}
}

void MyVeinsRSUApp::finish() {

}

void MyVeinsRSUApp::onWSM(BaseFrame1609_4 *frame) {
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) {
		return;
	}
	if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		if (reporterId == reporteeId)
			return; //not going to happen but failsafe for modification in veh app
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		if (archivedScope.count(reporteeId) == 1 && archivedScope[reporteeId]->count(msgId) == 1)
			return;
		bool staged = (stagedBasket->contains(reporteeId, msgId));
		if ((!staged) && (!currentBasket->contains(reporteeId)))
			initVehicle(reporteeId);
		vehStatsEntityCentric &reportee =
				staged ? *(stagedBasket->vehicles[reporteeId]) : *(currentBasket->vehicles[reporteeId]);
		if (reportee.reporters.count(reporterId) == 0)
			reportee.reporters[reporterId] = new reportsGist();
		reportsGist &reportersReportsGistforReportee = *(reportee.reporters[reporterId]);
		if (reportersReportsGistforReportee.messages.count(msgId))
			return;
		reportersReportsGistforReportee.messages[msgId] = foundValid;
		reportersReportsGistforReportee.reportedCount++;
		reportee.reportedCount++;
		if (foundValid) {
			reportee.reportedTrueCount++;
			reportersReportsGistforReportee.reportedTrueCount++;
		}
	}
}

void MyVeinsRSUApp::initVehicle(int vehId) {
	currentBasket->scope[vehId] = new intSet();
	currentBasket->vehicles[vehId] = new vehStatsEntityCentric();

}
void MyVeinsRSUApp::stageShift() {
	for (auto stagedScopeIt = stagedBasket->scope.begin(); stagedScopeIt != stagedBasket->scope.end();
			stagedScopeIt++) {
		if (archivedScope.find(stagedScopeIt->first) == archivedScope.end())
			archivedScope[stagedScopeIt->first] = new intSet();
		archivedScope[stagedScopeIt->first]->insert(stagedScopeIt->second->begin(), stagedScopeIt->second->end());
	}
	//TODO implement Mutex
	reportsBasket *archivedBasket = stagedBasket;
	stagedBasket = currentBasket;
	currentBasket = new reportsBasket();
	int_2_float secondaryScores = genarateSecondaryScores(archivedBasket);
	std::tr1::unordered_set<int> blacklistedReporters;
	populateBlacklistedReportersList(secondaryScores, blacklistedReporters);
	int_2_float primaryScores = genaratePrimaryScores_ReportsBased(archivedBasket, secondaryScores, blacklistedReporters);

	deleteBasket(archivedBasket);
}

int_2_float MyVeinsRSUApp::genaratePrimaryScores_ReportsBased(reportsBasket *basket, int_2_float secondaryScores,
		std::tr1::unordered_set<int> blacklist) {
	int_2_float rawScores;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		vehStatsEntityCentric &veh = *(basketIt->second);
		int totalReports = 0;
		int trueReports = 0;
		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
			if (blacklist.count(reporterIt->first) == 0) {
				trueReports += reporterIt->second->reportedTrueCount;
				totalReports += reporterIt->second->reportedCount;
			}
		}
		rawScores[vehId] = trueReports / totalReports;
	}
}
int_2_float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket,
		int_2_float secondaryScores, std::tr1::unordered_set<int> blacklist) {
	int_2_float rawScores;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		vehStatsEntityCentric &veh = *(basketIt->second);
		int trueMsgs = 0;
		int totalMsgs = 0;   //some messages may not have any reporter thats not blacklisted.
		for (auto msgId : *(basket->scope[vehId])) {
			int falseReports = 0;
			int trueReports = 0;
			for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
				if (blacklist.count(reporterIt->first) == 0 and reporterIt->second->messages.count(msgId) != 0) {
					if (reporterIt->second->messages[msgId])
						++trueReports;
					else ++falseReports;
				}
			}
			if (falseReports or trueReports) {
				++totalMsgs;
				if (trueReports > falseReports)
					++trueMsgs;
			}
		}
		if (totalMsgs)
			rawScores[vehId] = trueMsgs / totalMsgs;
		else rawScores[vehId] = -1;
	}
}
int_2_float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased(reportsBasket *basket, int_2_float secondaryScores,
		std::tr1::unordered_set<int> blacklist) {
	int_2_float rawScores;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		vehStatsEntityCentric &veh = *(basketIt->second);
		std::vector<float> messageScores;
		for (auto msgId : *(basket->scope[vehId])) {
			int trueReports = 0;
			int totalReports = 0;
			for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
				if (blacklist.count(reporterIt->first) == 0 and reporterIt->second->messages.count(msgId) != 0) {
					if (reporterIt->second->messages[msgId])
						++trueReports;
					++totalReports;
				}
			}
			if (totalReports)
				messageScores.insert(messageScores.end(), (float) trueReports / (float) totalReports);
		}
		if (messageScores.size())
			rawScores[vehId] = vectorMean(messageScores);
		else rawScores[vehId] = -1;
	}
}

void MyVeinsRSUApp::populateBlacklistedReportersList(int_2_float secondaryScores,
		std::tr1::unordered_set<int> &blacklist) {
//float SDfrom0ofSecScores = standardDeviation(mapValues(secondaryScores),0);
	std::vector<float> secScoresVec;
	mapValuestoContainer(secondaryScores, secScoresVec);
	float medianofSecScores = vectorMedian(secScoresVec);
//float SDofSecScores = standardDeviation(secScoresVec,medianofSecScores);
//float deviationThreshholdVal = SDofSecScores; //TODO get suggestion from Dr. Frank
	float medianAbsoluteDeviationfromMedian = medianAbsoluteDeviation(secScoresVec, medianofSecScores);

	float threshhold = medianofSecScores + (medianAbsoluteDeviationfromMedian);
//TODO get suggestion from Dr.Frank and/or Dr.Doss or VIT Statisitcs Professor
//alternative solution is to not create a blacklist and to scale the effect of a reporter on the primary score with the deviation of their sc score from 0.
	for (auto i : secondaryScores)
		if (i.second < threshhold)
			blacklist.insert(i.first);
}

int_2_float MyVeinsRSUApp::genarateSecondaryScores(reportsBasket *basket) {
	int_2_float meanWeightedSquareError;
	int_2_int numberOfVehsReportedOn;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		vehStatsEntityCentric &veh = *(basketIt->second);
		std::vector<float> rawscores;
		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt)
			rawscores.insert(rawscores.end(),
					(float) reporterIt->second->reportedTrueCount / (float) reporterIt->second->reportedCount);
		float median = vectorMedian(rawscores);
		if (ismultimodal(rawscores, median)) {
			//WaWaWeeWa!! Collusion detected!
			//maybe here we can do data centric analysis, do concensus on each message calc our own raw score
			throw 0;
			//return meanWeightedSquareError;
		}

		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
			float diff = median
					- (float) reporterIt->second->reportedTrueCount / (float) reporterIt->second->reportedCount;
			if (diff < 0)
				diff = -diff;
			if (meanWeightedSquareError.count(reporterIt->first) == 0) {
				meanWeightedSquareError[reporterIt->first] = 0;
				numberOfVehsReportedOn[reporterIt->first] = 0;
			}
			meanWeightedSquareError[reporterIt->first] += diff * diff * reporterIt->second->reportedCount; //MSE or MAE?  TSE for now.
			++numberOfVehsReportedOn[reporterIt->first];
		}
	}
	for (auto &it : meanWeightedSquareError)
		it.second /= numberOfVehsReportedOn[it.first];
	return meanWeightedSquareError;
}

void MyVeinsRSUApp::deleteBasket(reportsBasket *basket) { // put in destructor
	for (auto it = basket->scope.begin(); it != basket->scope.end(); ++it)
		delete it->second;
	for (auto it1 = basket->vehicles.begin(); it1 != basket->vehicles.end(); it1++) {
		for (auto it2 = it1->second->reporters.begin(); it2 != it1->second->reporters.end(); ++it2)
			delete it2->second;
		delete it1->second;
	}
	delete basket;
}

void MyVeinsRSUApp::handleSelfMsg(cMessage *msg) {
	if (false) {

	} else if (msg->getKind() == STAGE_SHIFT_EVT) {
		try{
		stageShift();}
		catch (...){
			return;
		}
		scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
	} else DemoBaseApplLayer::handleSelfMsg(msg);

}

void MyVeinsRSUApp::handlePositionUpdate(cObject *obj) {

}

