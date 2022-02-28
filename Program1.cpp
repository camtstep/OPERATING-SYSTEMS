// program1_cts96
// Programming Assignment 1
// Written by xxxxx & Cameron Stepanski
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <deque>
#include <algorithm>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define NUM_PLAYERS 3

using namespace std;

struct Deck
{
  deque<int> cards;
};

struct Player
{
  int id;
  deque<int> hand;
};

struct dealer_info
{
  Player* p[NUM_PLAYERS];
};
pthread_mutex_t mutexDeckUse;
pthread_cond_t status_next;
Deck* deck = new Deck;
bool winner_found = false;
int starter_turn = 0;
int round_num = 1;
int round_limit = 3;


void initialize_deck(Deck*&);
void* dealer_start (void*);
void* player_start(void*);
void take_turn (Player*&, Deck*&);
void discard_card(Player*&, Deck*&);
void player_draw(Player*&, Deck*&);
void player_print(const Player*);
void deck_print(const Deck*);
int draw_top_card();
void print_round_on_screen(const Player* const Player[], Deck*&);
void log_entry(const string);

int main(int argc, char *argv[])
{

  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " <seed_int> " << std::endl;
    return 1;
   }
  

  srand(atoi(argv[1]));
  Player* player1 = new Player;
  Player* player2 = new Player;
  Player* player3 = new Player;
  Player* players[NUM_PLAYERS] = {player1,player2,player3};
  dealer_info* dealer = new dealer_info;
  for (int n = 0; n < 3; n++) 
  {
    players[n]->id = n+1;
    dealer -> p[n] = players[n];
  }
  pthread_t threads[NUM_THREADS];
  pthread_mutex_init(&mutexDeckUse, NULL);

  for ( int r = 0; r<round_limit;r++)
  {
    initialize_deck(deck);
    pthread_create(&threads[0],NULL,dealer_start,(void *)dealer);
    pthread_join(threads[0],NULL);
    int rc;
    
    while (!winner_found)
    {
      for (int i = 0; i < NUM_PLAYERS; i++) 
      {
        rc = pthread_create(&threads[i+1], NULL, player_start, (void *)players[i]);
        if (rc) 
        {
          printf("[-]ERROR creating pthread %d\n", rc);
          exit(-1);
        }
      }
      for (int i = 1; i <= NUM_PLAYERS; i++)
      {
        if (pthread_join(threads[i], NULL)) 
        {
          fprintf(stderr, "[-]Error joining threads\n");
          exit(2);
        }
      }
    }

    print_round_on_screen(players,deck);
    for (int a= 0; a<NUM_PLAYERS; a++)
    {
      string exit_log = "PLAYER ";
      exit_log += to_string(players[a]->id);
      exit_log += ": exits round";
      int hand_size = players[a]->hand.size();
      for (int h = hand_size-1; h>=0; h--)
      {
        players[a]->hand.erase(players[a]->hand.begin()+h);
      }
      log_entry(exit_log);
    }
    winner_found = 0;
    round_num++;
  } 

  pthread_mutex_destroy(&mutexDeckUse);
  pthread_exit(NULL);
  }

void initialize_deck(Deck*& deck)
{
  delete deck;
  deck = new Deck;
  for (int i = 1; i<=13; i++)
  {
    for (int j = 0; j<4; j++)
    {
      deck->cards.push_back(i);
    }
  }
}

void* dealer_start(void* info)
{
  dealer_info* q = (dealer_info*) info;
  int deck_size = deck->cards.size();
  int position;
  starter_turn = round_num;
  int starter;

  log_entry("DEALER: shuffling");

  for(int i = 0; i< 7; i++)
  {
    for(int j = 0; j<52; j++)
    {
      position = rand() % 52;
      iter_swap(deck->cards.begin() + position, deck->cards.begin() + i);
    }
  }

  log_entry ("DEALER: dealing round");

  for (int a = 0; a<NUM_PLAYERS; a++)
  {
    starter = (round_num +a);
    starter = starter%3;
    if(starter == 1)
    {
      q ->p[0]->hand.push_back(draw_top_card());
    }
    else if(starter == 2)
    {
      q->p[1]->hand.push_back(draw_top_card());
    }
    else
    {
      q->p[2]->hand.push_back(draw_top_card());
    }
  }
  pthread_cond_broadcast(&status_next);
  pthread_exit(NULL);
}

void* player_start(void* player)
{
  Player* p = (Player*) player;
  while(winner_found == 0)
  { 
    pthread_mutex_lock(&mutexDeckUse); 
    while(winner_found == 0 && !(p->id == starter_turn || starter_turn == 0))
   { 
   pthread_cond_wait(&status_next, &mutexDeckUse);
   }
        if(winner_found == 0)
        {
          take_turn(p, deck);
        }
        pthread_mutex_unlock(&mutexDeckUse);
    }
   pthread_exit(NULL);
}

void take_turn(Player*& player, Deck*& deck)
{
  player_print(player);
  player_draw(player,deck);


  if(player->hand.at(0)==player->hand.at(1))
  {
    winner_found = 1;
    string winner_log = "PLAYER ";
    winner_log += to_string(player->id);
    winner_log += ": wins";
    player_print (player);
    log_entry (winner_log);
  }
  else
  {
    discard_card(player, deck);
    starter_turn++;
    if(starter_turn > NUM_PLAYERS)
    {
      starter_turn = 1;
    }
    deck_print(deck);
  }
  pthread_cond_broadcast(&status_next);
}

void discard_card(Player*& player, Deck*& deck)
{
  int discarded;
  Player* p = (Player*) player;
  int posi = rand() % 1;
  discarded = p->hand.at(posi);

  string discard_log = "Player ";
  discard_log += to_string(p->id);
  discard_log += ": discards ";
  discard_log += to_string(discarded);

  deck ->cards.push_back(discarded);
  p->hand.erase(p->hand.begin()+posi);

  log_entry(discard_log);
}

void player_draw (Player*& player, Deck*& deck)
{
  string draw_log = "PLAYER ";
  draw_log += to_string(player->id);
  draw_log += ": draws ";
  int new_card = draw_top_card();
  player->hand.push_back(new_card);
  draw_log += to_string(new_card);

  log_entry(draw_log);
}

void player_print(const Player* player)
{
  int hand_size = player->hand.size();

  string player_log = "Player ";
  player_log += to_string(player->id);
  player_log += ": hand ";

  for (int i = 0; i<hand_size; i++)
  {
    player_log += to_string (player->hand[i]);
    player_log += " ";
  }

  log_entry(player_log);
}

void deck_print(const Deck* deck)
{
  int deck_size = deck->cards.size();

  string deck_log = "DECK: ";



  for (int i = 0; i<deck_size; i++)
  {
    deck_log += to_string(deck->cards[i]);
    deck_log += " ";
  }

  log_entry(deck_log);

}

int draw_top_card()
{
  int card_drawn = deck->cards.front();
  deck->cards.pop_front();

  return card_drawn;
}

void print_round_on_screen(const Player* const players[], Deck*& deck)
{
  bool winner_check;
  
  for (int i = 0; i<3; i++)
  {
    int hand_size = players[i]->hand.size();
    winner_check = 0;
    if(hand_size==2)
    {
      winner_check =1;
    }
    printf("PLAYER %d:\n HAND ",players[i]->id);

    for (int h = 0; h< hand_size; h++)
    {
      printf("%d ",players[i]->hand[h]);
    } 
    printf ("\n WIN ");
    if(winner_check)
    {
      printf ("yes \n");
    }
    else
    {
      printf ("no \n");
    }

  }

  int deck_size = deck->cards.size();
  printf ("DECK: ");
  for(int j = 0; j<deck_size; j++)
  {
    printf("%d ",deck->cards.at(j));
  } 
  printf("\n");
}

void log_entry (string entry)
{
  ofstream file_output;
  ifstream file_input;
  file_input.open("game.log");
  file_output.open ("game.log",ios::app); // Append mode
  if (file_output.is_open()) 
  {
      file_output << entry << endl; // Writing data to file
  }
   file_input.close();
   file_output.close(); // Closing the file.
}
