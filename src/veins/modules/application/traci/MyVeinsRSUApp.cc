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
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
using namespace veins;
//TODO stage and archive messages by shifting them from one basket to another instead of shifting whole baskets if approach changes to data centric.
Define_Module(veins::MyVeinsRSUApp);

int MyVeinsRSUApp::node0id = 0; // for naming vehicles correctly.

//Static Variables (Common for all RSUs, workaround for inter-RSU communication.....not used for the vehicles of-course.)
std::mutex MyVeinsRSUApp::staticMemberAccessLock;
reportsBasket *MyVeinsRSUApp::currentBasket;
reportsBasket *MyVeinsRSUApp::stagedBasket;
int_2_intSet MyVeinsRSUApp::archivedScope; //  { senderId : { msgId : val } }
int_2_int2float MyVeinsRSUApp::scores;
intSet MyVeinsRSUApp::blacklist;
int_2_float MyVeinsRSUApp::reportDensity;
std::string MyVeinsRSUApp::blacklistCSV;
std::string MyVeinsRSUApp::broadcastDataCSV;
//STATS
int MyVeinsRSUApp::stageCounter;
int MyVeinsRSUApp::recievedReports;
intSet MyVeinsRSUApp::statsInitialisedVehicles;
intSet MyVeinsRSUApp::partialStatsInitialisedVehicles;
int_2_int MyVeinsRSUApp::reportsOn;
int_2_int MyVeinsRSUApp::reportsFrom;

void MyVeinsRSUApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
	} else if (stage == 1) {
		if (par("isMaster").boolValue())
			isMaster = true;
		else isMaster = false;
		recordScalar("my myId is:", myId);
		recordScalar("am I Master", (double) (isMaster ? 1 : -1));
		if ((myId + 6) > node0id)
			node0id = myId + 6; // for callibrating node nomenclature for statistics collection.
		logSplitFactor = par("logSplitFactor").intValue();
		logSplitLevel = par("logSplitLevel").intValue();
		logSplitSmallest = par("logSplitSmallest").intValue();
		logSplitSizes = calculatePowersAscending<std::vector<int>>(logSplitSmallest, logSplitFactor, logSplitLevel);
		myStage = 0;
		broadcastsSent = 0;
		broadcastsSentVector.setName("broadcastsSentVector");
		if (isMaster) {
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
			usableReportsInBasketCount.setName("usableReportsInBasketCount");
			reportersInStagedBasket.setName("reportersInStagedBasket");
			blacklistedReportersInStagedBasket.setName("blacklistedReportersInStagedBasket");

			vehiclesInCurrentScope.setName("vehiclesInCurrentScope");
			messagesInCurrentScope.setName("messagesInCurrentScope");
			reportsInCurrentBasket.setName("reportsInCurrentBasket");
			reportersInCurrentBasket.setName("reportersInCurrentBasket");
			scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
		}
	}
}

void MyVeinsRSUApp::finish() {
	if (isMaster) {
		stageShift();
		stageShift();
		stageShift();
		delete currentBasket;
		delete stagedBasket;
		//delete stageShiftEvt;
		//delete stageShiftEvt;
		for (auto id : statsInitialisedVehicles) {
			delete secondaryScoreVector[id];
			delete primaryScore_ReportsBasedVector[id];
			delete primaryScore_RAW_Vector[id];
			for (auto &i : primaryScore_MessagesBasedVector[id])
				delete i.second;
			for (auto &i : primaryScore_MessagesBased_MajorityAbsolutistVector[id])
				delete i.second;
			for (auto &i : primaryScore_MessagesBasedVector_WOBL[id])
				delete i.second;
			for (auto &i : primaryScore_MessagesBased_MajorityAbsolutistVector_WOBL[id])
				delete i.second;
			std::string idStr = std::to_string(id2num(id, node0id));
			recordScalar(std::string("reportsFrom").append(idStr).c_str(), reportsFrom[id]);
			recordScalar(std::string("reportsOn").append(idStr).c_str(), reportsOn[id]);
		}
		//for (auto &log : logs)
		//	delete log.second;
		//for (auto &log : logs_MA)
		//	delete log.second;
		//for (auto &log : logs_WOBL)
		//	delete log.second;
		//for (auto &log : logs_MA_WOBL)
		//	delete log.second;
	}
	for (auto &i : logs) {
		std::ofstream fout(std::string("R/RSUFinals/NodeAcc/").append(std::to_string(id2num(i.first, node0id))).c_str(),
				std::ios::out);
		fout << i.second.getOverallAvg();
		fout.close();
	}
}

void MyVeinsRSUApp::onWSM(BaseFrame1609_4 *frame) {
	recorder rec(
			std::string("RSU on WSM").append(std::to_string(myId)).append(std::to_string(simTime().raw())).append(
					".txt"));
	rec.record(0);
	if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		rec.record(1);
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		ingestReport(reporterId, reporteeId, msgId, foundValid);

	} else if (reportDumpMsg *wsm = dynamic_cast<reportDumpMsg*>(frame)) {
		rec.record(2);
		int reporteeId = wsm->getReporteeAddress();
		int senderId = wsm->getSenderAddress();
		intSet trueMsgs = csvToIntSet(std::string(wsm->getTrueMsgs()));
		intSet falseMsgs = csvToIntSet(std::string(wsm->getTrueMsgs()));
		for (auto msgId : trueMsgs)
			ingestReport(senderId, reporteeId, msgId, true);
		for (auto msgId : trueMsgs)
			ingestReport(senderId, reporteeId, msgId, false);
	}
	if ((!isMaster) && (myStage < stageCounter)) {
		rec.record(3);
		std::lock_guard<std::mutex> lock(staticMemberAccessLock);
		RSUBroadcast *rsucast = new RSUBroadcast();
		populateWSM(rsucast);
		rsucast->setName("RSU Broadcast");
		rsucast->setVehIdAndScoresCSV(broadcastDataCSV.c_str());
		rsucast->setBlacklistCSV(blacklistCSV.c_str());
		rsucast->setBroadcastId(stageCounter);
		sendDown(rsucast);
		broadcastsSentVector.record(++broadcastsSent);
		myStage = stageCounter;
	}
	rec.deletefile();
}
void MyVeinsRSUApp::ingestReport(int reporterId, int reporteeId, int msgId, bool foundValid) {
	std::lock_guard<std::mutex> lock(staticMemberAccessLock);
	if (isMaster) {
		for (auto id : partialStatsInitialisedVehicles)
			initVehicleStats(id);//workaround for omnet vector ownership issues. only master rsu node creates omnetpp stats vectors.
		//static variables and difference between master and non master rsu is due to the workaround to avoid inter RSU communication programming
		if (statsInitialisedVehicles.count(reporteeId) == 0)
			initVehicleStats(reporteeId);
		if (statsInitialisedVehicles.count(reporterId) == 0)
			initVehicleStats(reporterId);
	} else {
		if ((statsInitialisedVehicles.count(reporteeId) == 0)
				&& (partialStatsInitialisedVehicles.count(reporteeId) == 0))
			initVehicleStats(reporteeId);
		if ((statsInitialisedVehicles.count(reporterId) == 0)
				&& (partialStatsInitialisedVehicles.count(reporterId) == 0))
			initVehicleStats(reporterId);
	}

	if (reporterId == reporteeId)
		return; //not going to happen but failsafe for modification in veh app

	//STATS-----------------------------------------------
	++recievedReports;
	++reportsOn[reporteeId];
	++reportsFrom[reporterId];
	if (isMaster)
		recievedReportsVector.record(recievedReports);
	//----------------------------------------------------

	if ((archivedScope.count(reporteeId) != 0) && (archivedScope[reporteeId]->count(msgId) != 0))
		return;
	bool staged = (stagedBasket->contains(reporteeId, msgId));
	if (staged)
		stagedBasket->ingestReport(reporterId, reporteeId, msgId, foundValid);
	else currentBasket->ingestReport(reporterId, reporteeId, msgId, foundValid);
}

void MyVeinsRSUApp::initVehicleStats(int id) {
	if (isMaster) {
		if (statsInitialisedVehicles.count(id))
			return;
		std::string idstr = std::to_string(id2num(id, node0id));
		secondaryScoreVector[id] = new cOutVector(std::string("secondaryScoreVector-").append(idstr).c_str());
		primaryScore_ReportsBasedVector[id] = new cOutVector(
				std::string("primaryScore_ReportsBasedVector-").append(idstr).c_str());

		std::tr1::unordered_map<int, cOutVector*> primaryScoresVectors;

		for (auto size : logSplitSizes)
			primaryScoresVectors[size] =
					new cOutVector(
							std::string("primaryScore_MessageBased_SLICE").append(std::to_string(size)).append(
									std::string("-")).append(idstr).c_str());
		primaryScore_MessagesBasedVector[id] = primaryScoresVectors;

		for (auto size : logSplitSizes)
			primaryScoresVectors[size] = new cOutVector(
					std::string("primaryScore_MessagesBased_MajorityAbsolutistVector_SLICE").append(
							std::to_string(size)).append(std::string("-")).append(idstr).c_str());
		primaryScore_MessagesBased_MajorityAbsolutistVector[id] = primaryScoresVectors;

		for (auto size : logSplitSizes)
			primaryScoresVectors[size] = new cOutVector(
					std::string("primaryScore_MessagesBasedVector_WOBL_SLICE").append(std::to_string(size)).append(
							std::string("-")).append(idstr).c_str());
		primaryScore_MessagesBasedVector_WOBL[id] = primaryScoresVectors;

		for (auto size : logSplitSizes)
			primaryScoresVectors[size] = new cOutVector(
					std::string("primaryScore_MessagesBased_MajorityAbsolutistVector_WOBL_SLICE").append(
							std::to_string(size)).append(std::string("-")).append(idstr).c_str());
		primaryScore_MessagesBased_MajorityAbsolutistVector_WOBL[id] = primaryScoresVectors;

		primaryScore_RAW_Vector[id] = new cOutVector(
				std::string("primaryScore_RAW_Vector-").append(std::to_string((id))).c_str());

		if (partialStatsInitialisedVehicles.erase(id) == 0) {
			reportsOn[id] = 0;
			reportsFrom[id] = 0;
		}
		statsInitialisedVehicles.insert(id);
	} else {
		reportsOn[id] = 0;
		reportsFrom[id] = 0;
		partialStatsInitialisedVehicles.insert(id);
	}
}

void MyVeinsRSUApp::stageShift() {
	std::lock_guard<std::mutex> lock(staticMemberAccessLock);
	recorder rec(
			std::string("rsuStageShift recorder").append(std::to_string(myId)).append(std::to_string(myStage)).append(
					".txt"));
	rec.record(0);
	//STATS--------------------------
	vehiclesInArchivedScope.record(archivedScope.size());
	int sum = 0;
	for (auto i : archivedScope)
		sum += i.second->size();
	messagesInArchivedScope.record(sum);
	vehiclesInStagedScope.record(stagedBasket->vehiclesInScopeCount());
	messagesInStagedScope.record(stagedBasket->messagesInScopeCount());
	reportsInStagedBasket.record(stagedBasket->reportsInBasketCount());
	reportersInStagedBasket.record(stagedBasket->uniqueReportersCount());
	vehiclesInCurrentScope.record(currentBasket->vehiclesInScopeCount());
	messagesInCurrentScope.record(currentBasket->messagesInScopeCount());
	reportsInCurrentBasket.record(currentBasket->reportsInBasketCount());
	reportersInCurrentBasket.record(currentBasket->uniqueReportersCount());
	++stageCounter;
	++myStage;
	for (auto i : stagedBasket->vehicles)
		primaryScore_RAW_Vector[i.first]->record(
				(float) i.second.reportedTrueCount() / (float) i.second.reportedCount());
	//-------------------------------
	for (auto stagedScopeIt = stagedBasket->scope.begin(); stagedScopeIt != stagedBasket->scope.end();
			stagedScopeIt++) {
		if (archivedScope.count(stagedScopeIt->first) == 0)
			archivedScope[stagedScopeIt->first] = new intSet();
		archivedScope[stagedScopeIt->first]->insert(stagedScopeIt->second.begin(), stagedScopeIt->second.end());
	}
	rec.record(1);
	reportsBasket *archivedBasket = stagedBasket;
	stagedBasket = currentBasket;
	currentBasket = new reportsBasket();
	if (archivedBasket->scope.size()) { //first stage shift, nothing gets archived.
		for (auto i : archivedBasket->scope) {
			reportDensity[i.first] = (float) archivedBasket->vehicles[i.first].reportedCount()
					/ (float) archivedBasket->scope[i.first].size();
		}
		rec.record(2);
		int_2_float secondaryScores = genarateSecondaryScores(archivedBasket);
		for (auto &i : secondaryScores) {
			std::ofstream fout(
					std::string("R/RSUFinals/NodeSecScore/").append(std::to_string(id2num(i.first, node0id))).c_str(),
					std::ios::app);
			fout << i.second << "\n";
			fout.close();
		}
		intSet blacklistedReporters = generateBlacklistedReportersList(secondaryScores);
		usableReportsInBasketCount.record(stagedBasket->usableReportsInBasketCount(blacklistedReporters));
		int_2_float primaryScores_RB = genaratePrimaryScores_ReportsBased(archivedBasket, intSet(), logs_RB);
		int_2_int2float primaryScores_WOBL = genaratePrimaryScores_MessagesBased(archivedBasket, intSet(), logs_WOBL);
		int_2_int2float primaryScores = genaratePrimaryScores_MessagesBased(archivedBasket, blacklistedReporters, logs);
		int_2_int2float primaryScores_MA_WOBL = genaratePrimaryScores_MessagesBased_MajorityAbsolutist(archivedBasket,
				intSet(), logs_MA_WOBL);
		int_2_int2float primaryScores_MA = genaratePrimaryScores_MessagesBased_MajorityAbsolutist(archivedBasket,
				blacklistedReporters, logs_MA);
		scores = primaryScores;
		blacklist = blacklistedReporters;
		BroadcastResults(primaryScores, blacklistedReporters, stageCounter, par("liteNode").boolValue(), reportDensity);
		//STATS
		/*for (auto &i : logs) {
			int node = i.first;
			recorder evalrec(
					std::string("RSUEval/").append(std::to_string(stageCounter)).append("--").append(
							std::to_string(node)));
			evalrec.enable();
			for (auto &j : i.second.messages) {
				evalrec.recordString(
						std::string("\n").append(std::to_string(j.id)).append(",").append(
								std::to_string(j.truthValue)));
			}
		}*/
		recordPrimaryScores(logs, primaryScore_MessagesBasedVector);
		recordPrimaryScores(logs_WOBL, primaryScore_MessagesBasedVector_WOBL);
		recordPrimaryScores(logs_MA, primaryScore_MessagesBased_MajorityAbsolutistVector);
		recordPrimaryScores(logs_MA_WOBL, primaryScore_MessagesBased_MajorityAbsolutistVector_WOBL);
		blacklistedReportersInStagedBasket.record(blacklistedReporters.size());
	}
	rec.record(3);
	delete archivedBasket;
	rec.deletefile();
}
void MyVeinsRSUApp::recordPrimaryScores(int2VehMsgHistory &lgs,
		std::tr1::unordered_map<int, std::tr1::unordered_map<int, cOutVector*>> scoresVector) {
	for (auto veh : lgs) {
		int vehId = veh.first;
		int_2_float splitAvgs = veh.second.getSplitAvgs();
		if (splitAvgs.size()) {
			splitAvgs[logSplitSizes.at(logSplitSizes.size() - 1)] = veh.second.getOverallAvg();
			for (auto splitAvg : splitAvgs)
				scoresVector[vehId][splitAvg.first]->record(splitAvg.second);
		}
	}
}
int_2_float MyVeinsRSUApp::genaratePrimaryScores_ReportsBased(reportsBasket *basket, intSet blacklist,
		std::tr1::unordered_map<int, std::pair<int, int>> &logs) {
	int_2_float rawScores;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		if (logs.count(vehId))
			logs[vehId] = std::pair<int, int>(0, 0);
		vehStatsEntityCentric &veh = basketIt->second;
		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
			if (blacklist.count(reporterIt->first) == 0) {
				logs[vehId].first += reporterIt->second.reportedTrueCount();
				logs[vehId].second += reporterIt->second.reportedCount();
			}
		}
		rawScores[vehId] = (float) logs[vehId].first / (float) logs[vehId].second;
		primaryScore_ReportsBasedVector[vehId]->record(rawScores[vehId]);
	}
	return rawScores;
}
int_2_int2float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket,
		intSet blacklist, int2VehMsgHistory &logs) {
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		int_2_float msgAvgs = basket->getMsgAvgs(vehId, blacklist);
		if (msgAvgs.size()) {
			for (auto &i : msgAvgs)
				if (i.second > 0.5)
					i.second = 1;
				else i.second = 0;
			if (logs.count(vehId) == 0)
				logs[vehId] = vehMsgHistory(logSplitSmallest, logSplitFactor, logSplitLevel);
			logs[vehId].insert(msgAvgs);
		}
	}
	int_2_int2float primaryScores;
	intSet vehs = getMapKeys<intSet>(logs);
	for (auto vehId : vehs) {
		int_2_float splitAvgs = logs[vehId].getSplitAvgs();
		if (splitAvgs.size())
			primaryScores[vehId] = splitAvgs;
	}
	return primaryScores;
}
int_2_int2float MyVeinsRSUApp::genaratePrimaryScores_MessagesBased(reportsBasket *basket, intSet blacklist,
		int2VehMsgHistory &logs) {
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		int_2_float msgAvgs = basket->getMsgAvgs(vehId, blacklist);
		if (msgAvgs.size()) {
			if (logs.count(vehId) == 0)
				logs[vehId] = vehMsgHistory(logSplitSmallest, logSplitFactor, logSplitLevel);
			logs[vehId].insert(msgAvgs);
		}
	}
	int_2_int2float primaryScores;
	intSet vehs = getMapKeys<intSet>(logs);
	for (auto vehId : vehs) {
		int_2_float splitAvgs = logs[vehId].getSplitAvgs();
		if (splitAvgs.size())
			primaryScores[vehId] = splitAvgs;
	}
	return primaryScores;
}

intSet MyVeinsRSUApp::generateBlacklistedReportersList(int_2_float secondaryScores) {
	std::vector<float> secScoresVec = getMapValues<std::vector<float>>(secondaryScores);
	float medianofSecScores = vectorMedian(secScoresVec);
//float SDofSecScores = standardDeviation(secScoresVec,medianofSecScores);
//float deviationThreshholdVal = SDofSecScores; //TODO get suggestion from Dr. Frank

	float medianAbsoluteDeviationfromMedian = medianAbsoluteDeviation(secScoresVec, medianofSecScores);

	float threshhold = medianofSecScores + 2 * (medianAbsoluteDeviationfromMedian);
//TODO get suggestion from Dr.Frank and/or Dr.Doss or VIT Statisitcs Professor
//alternative solution is to not create a blacklist and to scale the effect of a reporter on the primary score with the deviation of their sc score from 0.
	intSet blacklist;
	for (auto i : secondaryScores)
		if (i.second > threshhold)
			blacklist.insert(i.first);
	//STATS
	secondaryScoreThreshhold.record(threshhold);

	return blacklist;
}

int_2_float MyVeinsRSUApp::genarateSecondaryScores(reportsBasket *basket) {
	int_2_float meanWeightedSquareError;
	//int_2_int numberOfVehsReportedOn;
	int_2_int numberOfReportsByReporter;
	for (auto basketIt = basket->vehicles.begin(); basketIt != basket->vehicles.end(); ++basketIt) {
		int vehId = basketIt->first;
		vehStatsEntityCentric &veh = basketIt->second;
		std::vector<float> rawscores;
		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt)
			rawscores.insert(rawscores.end(),
					((float) reporterIt->second.reportedTrueCount()) / ((float) reporterIt->second.reportedCount()));
		float median = vectorMedian(rawscores);
		if (ismultimodal(rawscores, median)) {
			//WaWaWeeWa!! Collusion detected!
			//maybe here we can do data centric analysis, do concensus on each message calc our own raw score
			//throw 0;
			//return meanWeightedSquareError;
		}

		for (auto reporterIt = veh.reporters.begin(); reporterIt != veh.reporters.end(); ++reporterIt) {
			float diff = median
					- ((float) reporterIt->second.reportedTrueCount()) / ((float) reporterIt->second.reportedCount());
			if (diff < 0)
				diff = -diff;
			if (meanWeightedSquareError.count(reporterIt->first) == 0) {
				meanWeightedSquareError[reporterIt->first] = 0;
				//numberOfVehsReportedOn[reporterIt->first] = 0;
				numberOfReportsByReporter[reporterIt->first] = 0;
			}
			meanWeightedSquareError[reporterIt->first] += diff * diff * (float) reporterIt->second.reportedCount(); //MSE or MAE?  MSE for now.

					//note2self: for values less than 1 MSE will be ofcourse lesser than MAE and but it doesn't matter,
					//values get reduced upon squaring but small values get much more reduced than big values get reduced,
					//so the MSE property of penalising larger errors is retained.
					//the question is: mse=sum(diff^2 * numReports) or mse= sum( (diff*numReports)^2 ) ?
					//i think the latter makes sense with the entitiy centric primary score calc and the former makes sense with the message based..maybe
					//maybe if we are weighing the raw scores with the sec score instead of blacklist approach. or maybe not since that is scaled with the number of messages not reports.
					//think this through in the bigger picture later.

			//++numberOfVehsReportedOn[reporterIt->first];
			numberOfReportsByReporter[reporterIt->first] += reporterIt->second.reportedCount();
		}
	}
	for (auto &it : meanWeightedSquareError) {
		//it.second /= numberOfVehsReportedOn[it.first];
		it.second /= (float) numberOfReportsByReporter[it.first];
		//STATS
		secondaryScoreVector[it.first]->record(it.second);
	}
	return meanWeightedSquareError;
}

void MyVeinsRSUApp::BroadcastResults(int_2_int2float scores, intSet blacklist, int broadcastId, bool litenode,
		int_2_float reportdensity) {
	std::string vId, score;
	broadcastDataCSV = std::string("");
	recorder rec(
			std::string("broadcastResults").append(std::to_string(myId)).append("-").append(
					std::to_string(simTime().raw())).append(".txt"));
	rec.disable();
	rec.record(0);
	for (auto veh : scores) {
		rec.recordString(std::string("\nin loop:").append(std::to_string(veh.first)).append("--"));
		rec.recordString(std::string("\n---------\n"));
		rec.recordString(broadcastDataCSV);
		rec.recordString(std::string("\n---------\n"));
		if (veh.second.size()) { // vehid, #messages, lastMsgId, (reportdensity), avg(splitSize0)...avg(splitSizeN)
			rec.record(1);
			broadcastDataCSV.append(std::to_string(veh.first).append(","));
			broadcastDataCSV.append(std::to_string((float) (logs[veh.first].messages.size())).append(","));
			broadcastDataCSV.append(std::to_string((float) (logs[veh.first].messages.at(0).id)).append(","));
			rec.record(2);
			if (litenode)
				broadcastDataCSV.append(std::to_string(reportdensity[veh.first]).append(","));
			rec.record(3);
			for (int i = 0; i < logSplitSizes.size(); ++i) {
				rec.recordString(std::string("\n\tInner loop:").append(std::to_string(i)).append("--"));
				if (veh.second.count(logSplitSizes.at(i)))
					broadcastDataCSV.append(std::to_string(veh.second[logSplitSizes.at(i)]).append(","));
				else broadcastDataCSV.append(std::to_string((float) -1).append(","));
				rec.record(4);
			}
		}
	}

	rec.recordString(std::string("\nout of loop\n----------\n"));
	rec.recordString(broadcastDataCSV);
	rec.recordString(std::string("\n---------\n"));
	blacklistCSV = "";
	for (auto i : blacklist)
		blacklistCSV.append(std::to_string(i).append(","));
	rec.record(5);
	if (broadcastDataCSV.length() > 2) {
		rec.record(6);
		RSUBroadcast *rsucast = new RSUBroadcast();
		populateWSM(rsucast);
		rsucast->setName("RSU Broadcast");
		rsucast->setVehIdAndScoresCSV(broadcastDataCSV.c_str());
		rsucast->setBlacklistCSV(blacklistCSV.c_str());
		rsucast->setBroadcastId(broadcastId);
		sendDown(rsucast);
		broadcastsSentVector.record(++broadcastsSent);
	}
}
void MyVeinsRSUApp::handleSelfMsg(cMessage *msg) {
	if (msg->getKind() == STAGE_SHIFT_EVT) {
		stageShift();
		scheduleAt(simTime() + stageShiftInterval, stageShiftEvt);
	} else DemoBaseApplLayer::handleSelfMsg(msg);
}

void MyVeinsRSUApp::handlePositionUpdate(cObject *obj) {

}

