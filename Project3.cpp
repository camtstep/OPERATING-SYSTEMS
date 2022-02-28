// CS 4328 Programming Assignment 3
// Authors: xxxxx and Cameron Stepanski

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <vector>
#include <deque>
#include <list>

using namespace std;

int const PAGE_REF = 100; // number of pages

void Create_Page_String (int);
void FIFO(int);
void LRU(int);
void OPT(int);

deque <string> myList;

int main() {

  int frame_size;
  int seed;

  cout << "Page frames: ";
  cin >> frame_size;
  cout << "Seed: ";
  cin >> seed;

  // create random page-reference string
  Create_Page_String(seed);

  // FIFO
  FIFO(frame_size);

  // LRU
  LRU(frame_size);

  // OPT
  OPT(frame_size);

  return 0;
} 

void Create_Page_String (int seed) {
  
  srand(seed); // used to create random numbers
  string generatedPage; // stores page int converted to string
  
  for(int i=0; i<PAGE_REF; i++) {
    int a = rand()%50; // creates ints from 0-49
    generatedPage = to_string(a); // converts int to string
    
    myList.push_back(generatedPage); // put page onto myList
  }
  cout << endl;
}

void FIFO(int frame_size) {
  
  int faults = 0; // keeps track of page faults
  list <string> frameList;
  list<string>::iterator iterator;
  deque <string> fifoList = myList; // myList for use in FIFO 
  bool exists;

  while(!fifoList.empty()) {

    if(frameList.size() < frame_size) {

      exists = false;

      list<string>::iterator iterator2;
      for(iterator2=frameList.begin(); iterator2!=frameList.end(); iterator2++) {
        if(*iterator2 == fifoList.front()) {
          exists = true;
        }
      }

      if(!exists) {
        frameList.push_back(fifoList.front());
        fifoList.pop_front();
        faults++;
      } else {
        fifoList.pop_front();
      }
    } else if(frameList.size() == frame_size) {
      bool inFrame = false;
      for(iterator = frameList.begin(); iterator!=frameList.end(); iterator++) {
        if(fifoList.front() == *iterator) {
          inFrame = true;
        }
      }
      if(inFrame == true) {
        fifoList.pop_front();
      } else {
      frameList.pop_front();
      frameList.push_back(fifoList.front());
      fifoList.pop_front();
      faults++;
      }
    }
  }
 
frameList.clear();

cout << "FIFO faults with " << frame_size << " frame(s): " <<  faults << endl;
}

void LRU(int frame_size) {
  
  int faults = 0; // keeps track of page faults
  deque<string> lruList = myList; // myList for use in LRU 
  vector<string> frameList;
  vector<string> poppedList;
  vector<string>::iterator iterator;
  int temp;
  bool exists;
  bool appearOnce;

  while(!lruList.empty()) {
    if(frameList.size() < frame_size) {
    
      exists = false;

      vector<string>::iterator iterator2;
      for(int i=0; i<frameList.size(); i++) {
        if(frameList.at(i) == lruList.front()) {
          exists = true;
        }
      }

      if(!exists) {
        frameList.push_back(lruList.front());
        poppedList.push_back(lruList.front());
        faults++;
      }  else {
        poppedList.push_back(lruList.front());
        lruList.pop_front();
      }
    } else {
      
      exists = false;

      vector<string>::iterator iterator2;
      for(int i=0; i<frameList.size(); i++) {
        if(frameList.at(i) == lruList.front()) {
          exists = true;
        }
      }
      
      if(exists) {
        poppedList.push_back(lruList.front());
        lruList.pop_front();
      } else {

        int i = (PAGE_REF-lruList.size()-1);
        string changePage;
        temp = frame_size;

        while(temp>0 && i>=0) {
          for(i=PAGE_REF-lruList.size()-1; i>=0; i--) {
            
            exists = false;

            vector<string>::iterator iterator2;
            for(int i=0; i<frameList.size(); i++) {
              if(frameList.at(i) == poppedList[i]) {
                exists = true;
              }
            }

            if(exists) {
              int num1 = PAGE_REF - lruList.size()-1;

              appearOnce = true;

              for(int j=num1; j<=i; j++) {
                if(poppedList[j]==poppedList[i]) {
                  appearOnce = false;
                }
              }


              if(appearOnce) {
                temp--;
                changePage = poppedList[i];
              } else {
                continue;
              }
            } else {
              continue;
            }
          }
        }

        for(iterator=frameList.begin(); iterator!=frameList.end(); iterator++) {
          if(changePage == *iterator) {
            frameList.erase(iterator);
            frameList.insert(iterator,lruList.front());
          } 
        }

        poppedList.push_back(lruList.front());
        lruList.pop_front();
        faults++; 
      }
    }
  }

  frameList.clear();
  poppedList.clear();

  cout <<  "LRU faults with " << frame_size << " frame(s): " <<  faults << endl;

}

void OPT(int frame_size) {
  
  int faults = 0; // keeps track of page faults
  deque<string> optList = myList; // myList for use in OPT 
  vector<string> frameList;
  vector<string>::iterator iterator;
  deque<string>::iterator iterator1;
  int temp;
  bool exists;
  bool appearOnce; 

  while(!optList.empty()){
    if(frameList.size()<frame_size) {

      exists = false;

      vector<string>::iterator iterator2;
      for(int i=0; i<frameList.size(); i++) {
        if(frameList.at(i) == optList.front()) {
          exists = true;
        }
      }

      if(!exists) {
        string temp;
        temp = optList.front();
        frameList.push_back(temp);
        optList.pop_front();
        faults++;
      } else {
        optList.pop_front();
      }
    } else {
      string changePage;
      
      exists = false;

      vector<string>::iterator iterator2;
      for(int i=0; i<frameList.size(); i++) {
        if(frameList.at(i) == optList.front()) {
          exists = true;
        }
      }
      if(exists) {
        optList.pop_front();
      } else {
        string changePage;
        temp = frame_size;
        bool useless = true;

        while(temp>0 && useless == true) {
          for(iterator1=optList.begin(); iterator1 != optList.end(); iterator1++) {
            
            exists = false;

            vector<string>::iterator iterator2;
            for(int i=0; i<frameList.size(); i++) {
              if(frameList.at(i) == *iterator1) {
                exists = true;
              }
            }
              if(exists){
                int num = optList.size();

                appearOnce = false;

                int n=0;
                for(int j=(num-1); j<PAGE_REF; j++){
                  if(optList[j] == *iterator1) {
                    n++;
                  }
                }
              

                if(n==1) {
                  appearOnce = true;
                }
              

                if(appearOnce) {
                  temp--;
                  changePage=*iterator1;
                }
              

              } else {
                changePage = *iterator1;
                useless = false;
                break;
              }
            }
          }

        // exchange page frame
        for(iterator=frameList.begin(); iterator!=frameList.end(); iterator++) {
          if(changePage==*iterator) {
            frameList.insert(iterator,optList.front());
            frameList.erase(iterator);
          }
        }

        optList.pop_front();
        faults++;
      }
    }
  }

  frameList.clear();

  cout <<  "OPT faults with " << frame_size << " frame(s): " << faults << endl;
}
