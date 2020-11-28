#include <cstdlib>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

#define CACHE_LINE_SIZE 32
#define KILOBYTE 1024

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
	
	for(auto input: in){
		unsigned long long currIndex = (input.addr >> 5) % cachelines;
		unsigned int currTag = input.addr / cachelines;
		if(cache[currIndex].validBit == 1 && cache[currIndex].tag == currTag){
			result.hits++;
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
		
		unsigned int timeLRU = in.size()+1;
		unsigned int indexLRU = 0;
		
		if(!match){
			currIndex = (input.addr >> 5) % indexSize;
			
			while(currIndex < 512){
				if(cache[currIndex].time < timeLRU){
					timeLRU = cache[currIndex].time;
					indexLRU = currIndex;
				}
				currIndex += indexSize;
			}
			
			cache[indexLRU].validBit = 1;
			cache[indexLRU].time = timer;
			cache[indexLRU].tag = currTag;
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
		unsigned long long currIndex = 0;
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
		
		currIndex = 0;
		
		if(!match){
		
			vector<int> temp = hotcold;
			int hotcoldIndex = 0;
			
			while(hotcoldIndex < 511){
				if(temp[hotcoldIndex] == 0){
					hotcoldIndex = 2*hotcoldIndex + 2;
				}
				else{
					hotcoldIndex = 2*hotcoldIndex + 1;
				}
			}
			hotcoldIndex -= 511;
			
			cache[hotcoldIndex].validBit = 1;
			cache[hotcoldIndex].tag = currTag;
			hotcold = changeHotCold(hotcoldIndex, hotcold);
		
		}
	
	}
	return result;

}

CacheResult setAssociateWriteMiss(int assoc, vector <input> in){
	CacheResult result = {0, in.size()};
	
	vector<cacheLine> cache;
	
	for(int i = 0; i < 512; i++){
		cacheLine c = {0, 0, 0};
		cache.push_back(c);
	}
	
	int indexSize = 512/assoc;
	
	int timer = 0;
	
	for(auto input : in){
		unsigned long long currIndex = (input.addr >> 5) % indexSize;
		unsigned int mask = (pow(2,32)-1);
		unsigned int currTag =  mask & (input.addr >> (int)(5 + log2(indexSize)));
		
		bool match = false;
		
		
		while(currIndex < 512 && (!match)){
			if(cache[currIndex].validBit == 0 && input.behave == "L"){
				cache[currIndex].tag = currTag;
				cache[currIndex].validBit = 1;
				cache[currIndex].time = timer;
				match = true;
			}
			//store instruction misses into the cache, written directly to memory
			else if(cache[currIndex].validBit == 1 && cache[currIndex].tag == currTag){
				result.hits++;
				cache[currIndex].time = timer;
				match = true;
			}
			currIndex += indexSize;
		}
		
		unsigned int LRUTime = in.size()+1;
		unsigned int LRUIndex = 0;
		
		if(match == false && input.behave == "L"){
			currIndex = (input.addr >> 5) % indexSize;
			while(currIndex < 512){
				if(cache[currIndex].time < LRUTime){
					LRUTime = cache[currIndex].time;
					LRUIndex = currIndex;
				}
				currIndex += indexSize;
			}
			
			cache[LRUIndex].validBit = 1;
			cache[LRUIndex].time = timer;
			cache[LRUIndex].tag = currTag;
		}
		timer++;
	}
	
		
	return result;
}

CacheResult setAssociativeNextLinePretech(int assoc, vector<input> in){
	CacheResult result = {0, in.size()};
	
	vector<cacheLine> cache;
	
	for(int i = 0; i < 512; i++){
		cacheLine c = {0, 0, 0};
		cache.push_back(c);
	}
	
	int indexSize = 512/assoc;
	
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
		
		unsigned int timeLRU = in.size()+1;
		unsigned int indexLRU = 0;
		if(!match){
			currIndex = (input.addr >> 5) % indexSize;
			
			while(currIndex < 512){
				if(cache[currIndex].time < timeLRU){
					timeLRU = cache[currIndex].time;
					indexLRU = currIndex;
				}
				currIndex += indexSize;
			}
			
			cache[indexLRU].validBit = 1;
			cache[indexLRU].time = timer;
			cache[indexLRU].tag = currTag;
		}
		
		//Next cache line 
		unsigned long long nextIndex = ((input.addr + 32) >> 5) % indexSize;
		unsigned int nextTag = mask & ((input.addr + 32) >> (int)(5 + log2(indexSize)));
		
		bool nextMatch = false;
		
		while(nextIndex < 512 && (!nextMatch)){
			if(cache[nextIndex].validBit == 0){
				cache[nextIndex].validBit = 1;
				cache[nextIndex].tag = nextTag;
				cache[nextIndex].time = timer;
				nextMatch = true;
			}
			
			else if(cache[nextIndex].validBit == 1 && cache[nextIndex].tag == nextTag){
				cache[nextIndex].time = timer;
				nextMatch = true;
			}
			
			nextIndex += indexSize;
		}
		
		unsigned int nextLRUTime = in.size() + 3;
		unsigned int nextLRUIndex = 0;
		
		if(!nextMatch){
			nextIndex = ((input.addr + 32) >> 5) % indexSize;
			
			while(nextIndex < 512){
				if(cache[nextIndex].time < nextLRUTime){
					nextLRUTime = cache[nextIndex].time;
					nextLRUIndex = nextIndex;
				}
				nextIndex+=indexSize;
			}
			cache[nextLRUIndex].validBit = 1;
			cache[nextLRUIndex].time = timer;
			cache[nextLRUIndex].tag = nextTag;
		
		}
		
		timer++;
		
	}
	
	return result;

}


CacheResult prefetchOnMiss(int assoc, vector<input> in){
	CacheResult result = {0, in.size()};
	
	vector<cacheLine> cache;
	
	for(int i = 0; i < 512; i++){
		cacheLine c = {0, 0, 0};
		cache.push_back(c);
	}
	
	int indexSize = 512/assoc;
	
	int timer = 0;
	
	for(auto input : in){
		unsigned long long currIndex = (input.addr >> 5) % indexSize;
		unsigned int mask = (pow(2,32)-1);
		unsigned int currTag =  mask & (input.addr >> (int)(5 + log2(indexSize)));
		
		bool match = false;
		bool fetch = false;
		
		while(currIndex < 512 && (!match)){
			if(cache[currIndex].validBit == 0){
				cache[currIndex].time = timer;
				cache[currIndex].validBit = 1;
				cache[currIndex].tag = currTag;
				match = true;
				fetch = true;
			}
			else if(cache[currIndex].validBit == 1 && cache[currIndex].tag == currTag){
				result.hits++;
				cache[currIndex].time = timer;
				match = true;
			}
			currIndex += indexSize;
		
		}
		
		
		unsigned int timeLRU = in.size()+1;
		unsigned int indexLRU = 0;
		if(!match){
			currIndex = (input.addr >> 5) % indexSize;
			
			while(currIndex < 512){
				if(cache[currIndex].time < timeLRU){
					timeLRU = cache[currIndex].time;
					indexLRU = currIndex;
				}
				currIndex += indexSize;
			}
			
			cache[indexLRU].validBit = 1;
			cache[indexLRU].time = timer;
			cache[indexLRU].tag = currTag;
			fetch = true;
		}
		
		if(fetch){
			unsigned long long nextIndex = ((input.addr + 32) >> 5) % indexSize;
			unsigned int nextTag = mask & ((input.addr + 32) >> (int)(5 + log2(indexSize)));
			
			bool nextMatch = false;
			
			while(nextIndex < 512 && (!nextMatch)){
				if(cache[nextIndex].validBit == 0){
					cache[nextIndex].time = timer;
					cache[nextIndex].validBit = 1;
					cache[nextIndex].tag = nextTag;
					nextMatch = true;
				}
				
				else if(cache[nextIndex].validBit == 1 && cache[nextIndex].tag == nextTag){
					cache[nextIndex].time = timer;
					nextMatch = true;
				}
				
				nextIndex += indexSize;
			
			}
			
			unsigned int nextLRUTime = in.size() + 3;
			unsigned int nextLRUIndex = 0;
			
			if(!nextMatch){
				nextIndex = ((input.addr + 32) >> 5) % indexSize;
				
				while(nextIndex < 512){
					if(cache[nextIndex].time < nextLRUTime){
						nextLRUTime = cache[nextIndex].time;
						nextLRUIndex = nextIndex;
					}
					nextIndex+=indexSize;
				}
				cache[nextLRUIndex].validBit = 1;
				cache[nextLRUIndex].time = timer;
				cache[nextLRUIndex].tag = nextTag;
			
			}
			
		}
		timer++;
	}
	return result;
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
	
	//Direct map
	vector<int> directMapSizes = {1, 4, 16, 32};
	for(auto size: directMapSizes){
		CacheResult result = directMapped(size, inputs);
		if (size != 32){
			output << result.hits << "," << result.access << "; ";
		}
		else{
			output << result.hits << "," << result.access << ";" << endl;
		}
	}

	//Set-Associative
	vector<int> associativities = {2, 4, 8, 16};
	for(auto associativity: associativities){
		CacheResult result = setAssociative(associativity, inputs);
		if(associativity != 16){
			output << result.hits << "," << result.access << "; ";
		}
		else{
			output << result.hits << "," << result.access << ";" << endl;
		}
	}
	
	//Fully associative with LRU replacement
	output << setAssociative(512, inputs).hits << "," << setAssociative(512, inputs).access << ";" << endl;
	
	//Fully associative with hot/cold
	output << fullyAssociative(inputs).hits << "," << fullyAssociative(inputs).access << ";" << endl;
	
	//
	for(auto associativity: associativities){
		CacheResult result = setAssociateWriteMiss(associativity, inputs);
		if(associativity != 16){
			output << result.hits << "," << result.access << "; ";
		}
		else{
			output << result.hits << "," << result.access << ";" << endl;
		}
	}
	
	for(auto associativity: associativities){
		CacheResult result = setAssociativeNextLinePretech(associativity, inputs);
		if(associativity != 16){
			output << result.hits << "," << result.access << "; ";
		}
		else{
			output << result.hits << "," << result.access << ";" << endl;
		}
	}
	
	for(auto associativity: associativities){
		CacheResult result = prefetchOnMiss(associativity, inputs);
		if(associativity != 16){
			output << result.hits << "," << result.access << "; ";
		}
		else{
			output << result.hits << "," << result.access << ";" << endl;
		}
	}
	
	infile.close();
	output.close();
	
}
