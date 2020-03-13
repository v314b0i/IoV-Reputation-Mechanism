package org.car2x.veins.modules.application.traci;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple MyVeinsApp extends DemoBaseApplLayer
{
    parameters:
    	@class(veins::MyVeinsApp);
    	string appName = default("My first Veins App!");
    	int percentageWithBadSendingAccuracy;
    	int badSendingAccuracyPercentage;
    	int goodSendingAccuracyPercentage;
    	int evaluatingAccuracyPercentage;
    	double messageInterval = default(5s) @unit(s);
    	double messageIntervalVarianceLimit = default(2000ms) @unit(ms);
}
