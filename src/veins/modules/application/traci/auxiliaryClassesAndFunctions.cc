#include "veins/modules/application/traci/auxiliaryClassesAndFunctions.h"
template<typename keytype, typename valtype, typename objtype> objtype mapValuestoContainer(
		std::tr1::unordered_map<keytype, valtype> map, objtype &obj) {
	for (auto it : map)
		obj.insert(obj.end(), it.second);
	return obj;
}
template<typename keytype, typename valtype, typename objtype> objtype mapKeystoContainer(
		std::tr1::unordered_map<keytype, valtype> map, objtype &obj) {
	for (auto it : map)
		obj.insert(obj.end(), it.first);
	return obj;
}
template<typename keytype, typename valtype> std::tr1::unordered_set<keytype> getKeySet(std::tr1::unordered_map<keytype, valtype> map) {
	std::tr1::unordered_set<keytype> keySet;
	for (auto it : map)
		keySet.insert(it.first);
	return keySet;
}
template<typename type> inline float vectorMedian(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	sort(vec.begin(), vec.end());
	return vec.at((int) (vec.size() / 2));
}
template<typename type> inline float vectorMean(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	float acc = 0;
	for (auto i : vec)
		acc += i;
	return acc / (float) vec.size();
}
template<typename type> inline float vectorMode(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	//appx algo written by me. may need to be replaced if the isMultimodal func's temporary logic isn't replaced or this is used elsewhere
	int hist[10] = { 0 }, hist2[9] = { 0 }, hmax = 0; //hist[i]=bin(10*i% to 10*(i+1)%) and hist2[i]=bin(5+10*i% to 5+10*(i+1)%)
	float hmaxi = 0;
	for (auto it : vec) {
		if ((it) >= 0.95) {
			if (++hist[9] > hmax) {
				hmax = hist[9];
				hmaxi = 0.945;
			}
		} else if ((it) <= 0.05) {
			if (++hist[0] > hmax) {
				hmax = hist[0];
				hmaxi = 0.045;
			}
		} else {
			if (++hist[(int) ((it) * 10)] > hmax) {
				hmax = hist[(int) ((it) * 10)];
				hmaxi = 0.045 + ((float) ((int) ((it) * 10))) / 10;
			}
			if (++hist2[(int) ((it - 0.05) * 10)] > hmax) {
				hmax = hist2[(int) ((it - 0.05) * 10)];
				hmaxi = 0.095 + ((float) ((int) ((it - 0.05) * 10))) / 10;
			}
		}
	}
	return hmaxi;
}
template<typename container> float inline standardDeviation(container obj, float centre) {
	float acc = 0;
	float diff;
	for (auto it : obj) {
		diff = it - centre;
		acc += diff * diff;
	}
	return sqrt(acc / (float) obj.size());
}
template<typename container> float inline medianAbsoluteDeviation(container obj, float centre) {
	std::vector<float> vec;
	for (auto it : obj)
		vec.insert(vec.end(), it > centre ? it - centre : centre - it);
	return vectorMedian(vec);
}

template<typename type> bool ismultimodal(std::vector<type> vec, float median) {
	//FIXME currently is an approximate testing function written by me. Should instead implement a proper test such as Hartigan's dip test
	//https://en.wikipedia.org/wiki/Multimodal_distribution#General_tests
	float mode = vectorMode(vec);
	float diffBetMedianAndMode = median > mode ? median - mode : mode - median;
	return diffBetMedianAndMode >= 0.20; // arbitrary cut-off. Use hartigans'd dip test and set cut off=0.1 on the p value.
	//should not consider the scores from nodes that had few total reports.
}
template<typename type> bool ismultimodal(std::vector<type> vec) {
	float median = vectorMedian(vec);
	return ismultimodal(vec, median);
}
intSet csvToIntSet(std::string csvstr) {
	std::tr1::unordered_set<int> set;
	std::string word;
	std::stringstream SS(csvstr.c_str());
	while (getline(SS, word, ','))
		set.insert(std::stoi(word));
	return set;
}
inline int uniqueReportersInBasket(reportsBasket basket) {
	intSet reporters;
	for (auto veh : basket.vehicles) {
		for (auto reporter : veh.second->reporters) {
			reporters.insert(reporter.first);
		}
	}
	return reporters.size();
}
inline int reportsInBasket(reportsBasket basket) {
	int reports = 0;
	for (auto veh : basket.vehicles) {
		for (auto reporter : veh.second->reporters) {
			reports += reporter.second->reportedCount;
		}
	}
	return reports;
}
inline int vehiclesInScope(int_2_intSet scope) {
	return scope.size();
}
inline int messagesInScope(int_2_intSet scope) {
	int count = 0;
	for (auto i : scope)
		count += i.second->size();
	return count;
}
