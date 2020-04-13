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
		//reportsDataOnVehicles = new int2vehStatsEntityCentric();
		//stagedReportsDataOnVehicles = new int2vehStatsEntityCentric(); //TODO shouldn't do this and insted should check if its still a nullptr when being used
	} else if (stage == 1) {

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

		bool staged = ((stagedBasket->scope.find(reporteeId)
				!= stagedBasket->scope.end())
				&& stagedBasket->scope[reporteeId]->count(msgId) != 0);
		/*if (vehicles.find(reporterId) == vehicles.end())
		 initVehicle(reporterId);
		 vehStatsEntityCentric &reporter = *(vehicles[reporterId]);*/

		if ((!staged)
				&& currentBasket->scope.find(reporteeId)
						== currentBasket->scope.end())
			initVehicle(reporteeId);
		vehStatsEntityCentric &reportee =
				staged ?
						*(stagedBasket->vehicles[reporteeId]) :
						*(currentBasket->vehicles[reporteeId]);
		if (reportee.reporters.find(reporterId) == reportee.reporters.end())
			reportee.reporters[reporterId] = new reportsGist();
		reportsGist &reportersReportsGistforReportee =
				*(reportee.reporters[reporterId]);
		if (reportersReportsGistforReportee.messages.find(msgId)
				!= reportersReportsGistforReportee.messages.end())
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
	for (auto stagedScopeIt = stagedBasket->scope.begin();
			stagedScopeIt != stagedBasket->scope.end(); stagedScopeIt++) {
		if (archivedScope.find(stagedScopeIt->first) == archivedScope.end())
			archivedScope[stagedScopeIt->first] = new intSet();
		archivedScope[stagedScopeIt->first]->insert(
				stagedScopeIt->second->begin(), stagedScopeIt->second->end());
	}
	//TODO implement Mutex
	reportsBasket *archivedBasket = stagedBasket;
	stagedBasket = currentBasket;
	currentBasket = new reportsBasket();

	deleteBasket(archivedBasket);
}
void MyVeinsRSUApp::genarateSecondaryScores() {
 //perform diptest for reports on each veh reports, if not multimodal, calculate score for each. report outliers.
 //if multimodal (collusion) : TBD
}

float MyVeinsRSUApp::calcSecondaryScore(int vehId) {
 //calculate score based on disimilarity to concensus on node.
}

void MyVeinsRSUApp::deleteBasket(reportsBasket *basket) {
	for (auto it = basket->scope.begin(); it != basket->scope.end(); ++it)
		delete it->second;
	for (auto it1 = basket->vehicles.begin(); it != basket->vehicles.end();
			it++) {
		for (auto it2 = it1->second->reporters.begin();
				it2 != it1->second->reporters.end(); ++it2)
			delete it2->second;
		delete it1->second;
	}
	delete basket;
}



void MyVeinsRSUApp::handleSelfMsg(cMessage *msg) {

}

void MyVeinsRSUApp::handlePositionUpdate(cObject *obj) {

}

