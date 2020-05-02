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

package org.car2x.veins.modules.application.traci;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple MyVeinsNodeApp_Lite extends DemoBaseApplLayer
{
    parameters:
    	@class(veins::MyVeinsNodeApp_Lite);
    	string appName = default("MyVeinsNodeApp_Lite");
    	int logSplitFactor = default(5);
		int logSplitLevel = default(4);
		int logSplitSmallest = default(10);
    	int percentageWithBadSendingAccuracy;
    	int badSendingAccuracyPercentage;
    	int goodSendingAccuracyPercentage;
    	int percentageOfInfoEvaluatable;
    	int percentageWithBadEvaluatingAccuracy;
    	int badEvaluatingAccuracyPercentage;
    	int goodEvaluatingAccuracyPercentage;
    	double messageInterval = default(5s) @unit(s);
    	double messageIntervalVarianceLimit = default(2000ms) @unit(ms);
    	double reportGenTime = default(7s) @unit(s);
    	double reportGenTimeVarianceLimit	= default(5000ms) @unit(ms);
    	double requestResponseDelay = default(1s) @unit(s);
		double requestResponseDelayVarianceLimit = default(500ms) @unit(ms);
		double requestDelay = default(1s) @unit(s);
		double requestDelayVarianceLimit  = default(500ms) @unit(ms);
		bool withoutReportDumpSharing = default(false);
		bool isNode0 = default(false);
}
