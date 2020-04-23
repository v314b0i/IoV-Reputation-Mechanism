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

		//STATS
		stageCounter = 0;
		recievedReports = 0;
		recievedReportsVector.setName("recievedReportsVector");
		secondaryScoreThreshhold.setName("secondaryScoreThreshhold");

		vehiclesInArchivedScope.setName("vehiclesInArchivedScope");
		messagesInArchivedScope.setName("messagesInArchivedScope");

		vehiclesInStagedScope.setName("vehiclesInStagedScope");
		messagesInStagedScope.setName("messagesInStagedScope");
		reportsInStagedBasket.setName("reportsInStagedBasket");
		reportersInStagedBasket.setName("reportersInStagedBasket");
		blacklistedReportersInStagedBasket.setName("blacklistedReportersInStagedBasket");

		vehiclesInCurrentScope.setName("vehiclesInCurrentScope");
		messagesInCurrentScope.setName("messagesInCurrentScope");
		reportsInCurrentBasket.setName("reportsInCurrentBasket");
		reportersInCurrentBasket.setName("reportersInCurrentBasket");
	} else if (stage == 1) {
		scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
	}
}

void MyVeinsRSUApp::finish() {
	stageShift();
	deleteBasket(currentBasket);
	deleteBasket(stagedBasket);
	//delete stageShiftEvt;
	//delete stageShiftEvt;
	for (auto id : statsInitialisedVehicles) {
		delete secondaryScoreVector[id];
		delete primaryScore_ReportsBasedVector[id];
		delete primaryScore_MessagesBased_MajorityAbsolutistVector[id];
		delete primaryScore_MessagesBasedVector[id];
		delete primaryScore_RAW_Vector[id];
		std::string idStr = std::to_string((id - 15) / 6);
		recordScalar(std::string("reportsFrom").append(idStr).c_str(), reportsFrom[id]);
		recordScalar(std::string("reportsOn").append(idStr).c_str(), reportsOn[id]);
	}
}

void MyVeinsRSUApp::onWSM(BaseFrame1609_4 *frame) {
	//if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) {
	//	return;
	//}
	if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		ingestReport(reporterId, reporteeId, msgId, foundValid);

	} else if (reportDumpMsg *wsm = dynamic_cast<reportDumpMsg*>(frame)) {
		int reporteeId = wsm->getReporteeAddress();
		int senderId = wsm->getSenderAddress();
		intSet trueMsgs(csvToIntSet(wsm->getTrueMsgs()));
		intSet falseMsgs(csvToIntSet(wsm->getTrueMsgs()));
		for (auto msgId : trueMsgs)
			ingestReport(senderId, reporteeId, msgId, true);
		for (auto msgId : trueMsgs)
			ingestReport(senderId, reporteeId, msgId, false);
	}
}
void MyVeinsRSUApp::ingestReport(int reporterId, int reporteeId, int msgId, bool foundValid) {
	if (statsInitialisedVehicles.count(reporteeId) == 0)
		initVehicleStats(reporteeId);
	if (statsInitialisedVehicles.count(reporterId) == 0)
		initVehicleStats(reporterId);
	if (reporterId == reporteeId)
		return; //not going to happen but failsafe for modification in veh app

	//STATS-----------------------------------------------
	++recievedReports;
	++reportsOn[reporteeId];
	++reportsFrom[reporterId];
	recievedReportsVector.record(recievedReports);
	//----------------------------------------------------

	if ((archivedScope.count(reporteeId) != 0) && (archivedScope[reporteeId]->count(msgId) != 0))
		return;
	bool staged = (stagedBasket->contains(reporteeId, msgId));
	if ((!staged) && (!(currentBasket->contains(reporteeId))))
		addVehicletoCurrentBasket(reporteeId);
	if ((!staged) && (!(currentBasket->contains(reporteeId, msgId))))
		currentBasket->scope[reporteeId]->insert(msgId);
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
void MyVeinsRSUApp::addVehicletoCurrentBasket(int vehId) {
	currentBasket->scope[vehId] = new intSet();
	currentBasket->vehicles[vehId] = new vehStatsEntityCentric();
}
void MyVeinsRSUApp::initVehicleStats(int id) {
	statsInitialisedVehicles.insert(id);

	secondaryScoreVector[id] = new cOutVector(
			std::string("secondaryScoreVector-").append(std::to_string((id - 15) / 6)).c_str());
	primaryScore_ReportsBasedVector[id] = new cOutVector(
			std::string("primaryScore_ReportsBasedVector-").append(std::to_string((id - 15) / 6)).c_str());
	primaryScore_MessagesBased_MajorityAbsolutistVector[id] =
			new cOutVector(
					std::string("primaryScore_MessagesBased_MajorityAbsolutistVector-").append(
							std::to_string((id - 15) / 6)).c_str());
	primaryScore_MessagesBasedVector[id] = new cOutVector(
			std::string("primaryScore_MessagesBasedVector-").append(std::to_string((id - 15) / 6)).c_str());
	primaryScore_RAW_Vector[id] = new cOutVector(
			std::string("primaryScore_RAW_Vector-").append(std::to_string((id - 15) / 6)).c_str());
	reportsOn[id] = 0;
	reportsFrom[id] = 0;
}

void MyVeinsRSUApp::stageShift() {
	//STATS--------------------------
	/*std::string stgNum = std::string("Stage").append(std::to_string(stageCounter).append(": "));
	recordScalar(std::string(stgNum).append("#vehicles in ArchivedScope").c_str(), vehiclesInScope(archivedScope));
	recordScalar(std::string(stgNum).append("#messages in ArchivedScope").c_str(), messagesInScope(archivedScope));

	recordScalar(std::string(stgNum).append("#vehicles in StagedScope").c_str(), vehiclesInScope(stagedBasket->scope));
	recordScalar(std::string(stgNum).append("#messages in StagedScope").c_str(), messagesInScope(stagedBasket->scope));
	recordScalar(std::string(stgNum).append("#reports  in StagedBasket").c_str(), reportsInBasket(*stagedBasket));
	recordScalar(std::string(stgNum).append("#reportersin StagedBasket").c_str(),
			uniqueReportersInBasket(*stagedBasket));

	recordScalar(std::string(stgNum).append("#vehicles in CurrentScope").c_str(),
			vehiclesInScope(currentBasket->scope));
	recordScalar(std::string(stgNum).append("#messages in CurrentScope").c_str(),
			messagesInScope(currentBasket->scope));
	recordScalar(std::string(stgNum).append("#reports  in CurrentBasket").c_str(), reportsInBasket(*currentBasket));
	recordScalar(std::string(stgNum).append("#reportersin CurrentBasket").c_str(),
			uniqueReportersInBasket(*currentBasket));*/
	vehiclesInArchivedScope.record(vehiclesInScope(archivedScope));
	messagesInArchivedScope.record(messagesInScope(archivedScope));
	vehiclesInStagedScope.record(vehiclesInScope(stagedBasket->scope));
	messagesInStagedScope.record(messagesInScope(stagedBasket->scope));
	reportsInStagedBasket.record(reportsInBasket(*stagedBasket));
	reportersInStagedBasket.record(uniqueReportersInBasket(*stagedBasket));
	vehiclesInCurrentScope.record(vehiclesInScope(currentBasket->scope));
	messagesInCurrentScope.record(messagesInScope(currentBasket->scope));
	reportsInCurrentBasket.record(reportsInBasket(*currentBasket));
	reportersInCurrentBasket.record(uniqueReportersInBasket(*currentBasket));
	++stageCounter;
	for (auto i : stagedBasket->vehicles)
		primaryScore_RAW_Vector[i.first]->record((float) i.second->reportedTrueCount / (float) i.second->reportedCount);
	//-------------------------------
	for (auto stagedScopeIt = stagedBasket->scope.begin(); stagedScopeIt != stagedBasket->scope.end();
			stagedScopeIt++) {
		if (archivedScope.count(stagedScopeIt->first) == 0)
			archivedScope[stagedScopeIt->first] = new intSet();
		archivedScope[stagedScopeIt->first]->insert(stagedScopeIt->second->begin(), stagedScopeIt->second->end());
	}
	//TODO implement Mutex
	reportsBasket *archivedBasket = stagedBasket;
	stagedBasket = currentBasket;
	currentBasket = new reportsBasket();
	if (archivedBasket->scope.size()) { //first stage shift, nothing gets archived.
		int_2_float secondaryScores = genarateSecondaryScores(archivedBasket);
		intSet blacklistedReporters;
		populateBlacklistedReportersList(secondaryScores, blacklistedReporters);
		genaratePrimaryScores_ReportsBased(archivedBasket, secondaryScores, blacklistedReporters);
		genaratePrimaryScores_MessagesBased_MajorityAbsolutist(archivedBasket, secondaryScores, blacklistedReporters);
		genaratePrimaryScores_MessagesBased(archivedBasket, secondaryScores, blacklistedReporters);
		//STATS
		blacklistedReportersInStagedBasket.record(blacklistedReporters.size());
	}
	deleteBasket(archivedBasket);
}

int_2_float MyVeinsRSUApp::genaratePrimaryScores_ReportsBased(reportsBasket *basket, int_2_float secondaryScores,
		intSet blacklist) {
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
		if (totalReports)
			rawScores[vehId] = (float) trueReports / (float) totalReports;
	}
	for (auto i : rawScores)
		primaryScore_ReportsBasedVector[i.first]->record(i.second);
	return rawScores;
}
int_2_float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket,
		int_2_float secondaryScores, intSet blacklist) {
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
			rawScores[vehId] = (float) trueMsgs / (float) totalMsgs;
	}
	for (auto i : rawScores)
		primaryScore_MessagesBased_MajorityAbsolutistVector[i.first]->record(i.second);
	return rawScores;
}
int_2_float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased(reportsBasket *basket, int_2_float secondaryScores,
		intSet blacklist) {
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
	}
	for (auto i : rawScores)
		primaryScore_MessagesBasedVector[i.first]->record(i.second);
	return rawScores;
}

void MyVeinsRSUApp::populateBlacklistedReportersList(int_2_float secondaryScores, intSet &blacklist) {
	std::vector<float> secScoresVec;
	mapValuestoContainer(secondaryScores, secScoresVec);
	float medianofSecScores = vectorMedian(secScoresVec);
//float SDofSecScores = standardDeviation(secScoresVec,medianofSecScores);
//float deviationThreshholdVal = SDofSecScores; //TODO get suggestion from Dr. Frank

	float medianAbsoluteDeviationfromMedian = medianAbsoluteDeviation(secScoresVec, medianofSecScores);

	float threshhold = medianofSecScores + 2 * (medianAbsoluteDeviationfromMedian);
//TODO get suggestion from Dr.Frank and/or Dr.Doss or VIT Statisitcs Professor
//alternative solution is to not create a blacklist and to scale the effect of a reporter on the primary score with the deviation of their sc score from 0.
	for (auto i : secondaryScores)
		if (i.second > threshhold)
			blacklist.insert(i.first);

	//STATS
	secondaryScoreThreshhold.record(threshhold);
}

int_2_float MyVeinsRSUApp::genarateSecondaryScores(reportsBasket *basket) {
	int_2_float meanWeightedSquareError;
	//int_2_int numberOfVehsReportedOn;
	int_2_int numberOfReportsByReporter;
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
			//throw 0;
			//return meanWeightedSquareError;
		}

		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
			float diff = median
					- (float) reporterIt->second->reportedTrueCount / (float) reporterIt->second->reportedCount;
			if (diff < 0)
				diff = -diff;
			if (meanWeightedSquareError.count(reporterIt->first) == 0) {
				meanWeightedSquareError[reporterIt->first] = 0;
				//numberOfVehsReportedOn[reporterIt->first] = 0;
				numberOfReportsByReporter[reporterIt->first] = 0;
			}
			meanWeightedSquareError[reporterIt->first] += diff * diff * reporterIt->second->reportedCount; //MSE or MAE?  MSE for now.

			//note2self: for values less than 1 MSE will be ofcourse lesser than MAE and but it doesn't matter,
			//values get reduced upon squaring but small values get much more reduced than big values get reduced,
			//so the MSE property of penalising larger errors is retained.
			//the question is: mse=sum(diff^2 * numReports) or mse= sum( (diff*numReports)^2 ) ?
			//i think the latter makes sense with the entitiy centric primary score calc and the former makes sense with the message based..maybe
			//maybe if we are weighing the raw scores with the sec score instead of blacklist approach. or maybe not since that is scaled with the number of messages not reports.
			//think this through in the bigger picture later.

			//++numberOfVehsReportedOn[reporterIt->first];
			numberOfReportsByReporter[reporterIt->first]+=reporterIt->second->reportedCount;
		}
	}
	for (auto &it : meanWeightedSquareError) {
		//it.second /= numberOfVehsReportedOn[it.first];
		it.second /= numberOfReportsByReporter[it.first];
		//STATS
		secondaryScoreVector[it.first]->record(it.second);
	}
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
	if (msg->getKind() == STAGE_SHIFT_EVT) {
		stageShift();
		scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
	} else DemoBaseApplLayer::handleSelfMsg(msg);

}

void MyVeinsRSUApp::handlePositionUpdate(cObject *obj) {

}

