#include <cstdlib>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

#define CACHE_LINE_SIZE 32
#define KILOBYTE	1024

struct input{
	string behave;
	unsigned long long addr;
};

struct cacheLineDirect{
	int validBit;
	unsigned long long tag;
};

struct cacheLine{
	int time;
	int validBit;
	unsigned long long tag;
};

struct CacheResult{
	int hits;
	unsigned long long access;
};
	
CacheResult directMapped(int cachesize, vector<input> in){

	CacheResult result = {0, in.size()};
	vector<cacheLineDirect> cache;
	int cachelines = (cachesize * KILOBYTE) / CACHE_LINE_SIZE;
	
	for (int i = 0; i < cachelines; i++){
		cacheLineDirect c = {0, 0};
		cache.push_back(c); 
	}
	
	for(int i = 0; i < in.size(); i++){
		unsigned long long currIndex = (in[i].addr >> 5) % (int)pow(2, log2(cachelines));
		unsigned int currTag = in[i].addr / pow(2, log2(cachelines));
		if(cache[currIndex].validBit == 1){
			if(cache[currIndex].tag == currTag){
				result.hits++;
			}
		}
		else{
			cache[currIndex].tag = currTag;
			cache[currIndex].validBit = 1;
		}
	}
	
	return result;

	
}

CacheResult setAssociative(int assoc, vector<input> in){

	CacheResult result = {0, in.size()};
	vector<cacheLine> cache;
	
	for(int i = 0; i < 512; i++){
		cacheLine c = {0, 0, 0};
		cache.push_back(c);
	}
	
	int indexSize = 512 / assoc;
	
	int timer = 0;
	
	for(auto input : in){
		unsigned long long currIndex = (input.addr >> 5) % indexSize;
		unsigned int mask = (pow(2,32)-1);
		unsigned int currTag =  mask & (input.addr >> (int)(5 + log2(indexSize)));
		
		bool match = false;
		
		while(currIndex<512 && (!match)){
			if(cache[currIndex].validBit == 1){
				if(cache[currIndex].tag == currTag){
					result.hits++;
					cache[currIndex].time = timer;
					match = true;
				}
			}
			else if (cache[currIndex].validBit == 0){
				cache[currIndex].validBit = 1;
				cache[currIndex].time = timer;
				cache[currIndex].tag = currTag;
				match = true;
			
			}
			
			currIndex += indexSize;
		
		}
		
		unsigned int LRUTime = in.size()+1;
		unsigned int LRUIndex = 0;
		
		if(!match){
			currIndex = (input.addr >> 5) % indexSize;
			
			while(currIndex < 512){
				if(cache[currIndex].time < LRUTime){
					LRUTime = cache[currIndex].time;
					LRUIndex = currIndex;
				}
				currIndex+=indexSize;
			}
			cache[LRUIndex].validBit = 1;
			cache[LRUIndex].time = timer;
			cache[LRUIndex].tag = currTag;
		}
		timer++;
	}
	
	return result;
}

vector<int> changeHotCold(int index, vector<int> &hotcold){
	int hotcoldIndex = index+511;
	while(hotcoldIndex > 0){
		if(hotcoldIndex % 2 == 0){
			hotcold[(hotcoldIndex-1)/2] = 1;
		}
		else{
			hotcold[(hotcoldIndex-1)/2] = 0;
		}
		hotcoldIndex = (hotcoldIndex-1)/2;
	}
	return hotcold;
}

CacheResult fullyAssociative(vector<input> in){
	CacheResult result = {0, in.size()};
	
	vector<cacheLine> cache;
	
	for(int i = 0; i < 512; i++){
		cacheLine c = {0, 0, 0};
		cache.push_back(c);
	}
	
	vector<int> hotcold(511,0);
	
	for(auto input: in){
		unsigned long long currIndex = 0
		unsigned currTag = input.addr >> 5;
		
		bool match = false;
		
		while(currIndex < 512 && (!match)){
			if(cache[currIndex].validBit == 1){
				if(cache[currIndex].tag == currTag){
					result.hits++;
					match = true;
					hotcold = changeHotCold(currIndex, hotcold);
				}
			}
			currIndex++;
		}
		
		if(!match){
		
			vector<int> temp = hotcold;
			int hotcoldIndex = 0
			
			while(hotcoldIndex < 511){
				if(temp[hotcoldIndex] == 0){
					hotcoldIndex = 2*hotcoldIndex + 2;
				}
				else{
					hotcoldIndex = 2*hotcoldIndex + 1;
				}
			}
			hotcoldIndex -= 511;
			
			cache[hotcoldIndex].validBit = 1
			cache[hotcoldIndex].tag = currTag;
			hotcold = changeHotCold(bootIndex, hotcold);
		
		}
	
	}
	in
	return result;

}

CacheResult setAssociateWriteMiss(vector <input> in){
	

}




int main(int argc, char** argv){
	ifstream infile(argv[1]);
	ofstream output(argv[2]);
	vector<input> inputs;
		string behavior;
	unsigned long long target;
	while(infile >> behavior >> std::hex >> target) {
		input i;
		i.behave = behavior;
		i.addr = target;
		inputs.push_back(i);
	}
}
