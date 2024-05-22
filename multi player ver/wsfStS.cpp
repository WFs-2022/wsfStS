/***********************************************/
/* File Name: wsfStS.cpp (wsf Slay the Spire)  */
/* Creator: CVE-2020-0796 (bilibili)           */
/* Date: 2024/05/18                            */
/* Comment: A small simulate of StS but of CLI */
/* P.S. It's the MULTI PLAYER version          */
/* LICENSE: it's under GPLv3 License           */
/***********************************************/

#include <bits/stdc++.h>
#include <conio.h>
#include  <winsock2.h>
#include  <ws2tcpip.h>
#include <windows.h>
#include <ctime>
#include <cstdio>
using namespace std;

// define for windows sockets
#define DEFAULT_BUFLEN 512

// Declare of usual useful functions, copied from my other CPP games. Maybe they'll be used in the future
void init();
//POINT GetMousePos();
//void gotoxy(int x, int y);

// Declare of functions for windows socket
int winsockInit();
void sendString(string sendString);
string recvString();

// Card structure
struct Card {
    string name, type; // type is from {"phy"(physical), "ablt"(abilities), "mag"(magical), "psy"(psychological)}
    int cost, obj, damag, block, heal, draw, power, aglty, sheld, times; // 'times' for repeating times
//    obj: 0->choose; 1->player, and so on
    string text;
};

// Map for str2Card (deck to player.deck)
map<string, Card> cardDict = {
							  {"Strike", {"Strike", "phy", 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"}}, 
							  {"Strikeee", {"Strikeee", "phy", 1, 0, 2, 0, 0, 0, 0, 0, 0, 3, "Deal 2 damage to a player 3 times"}}, 
							  {"Mana_hit", {"Mana hit", "phy", 2, 0, 6, 0, 0, 0, 0, 0, 0, 1, "Deal 7 damage, and you'll get 1 more mana next turn"}}, 
							  {"Crash", {"Crash", "phy", 1, 0, 3, 0, 0, 1, 0, 0, 0, 1, "Deal 3 damage and draw 1 card"}}, 
							  {"Defend", {"Defend", "phy", 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"}}, 
							  {"Dodge", {"Dodge", "phy", 1, 1, 0, 4, 0, 2, 0, 0, 0, 1, "Get 4 block and draw 2 cards"}}, 
							  {"Power_up", {"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, "Add 2 strength"}}, 
							  {"Armor_up", {"Armor up" , "ablt", 1, 1, 0, 0, 0, 0, 0, 3, 0, 1, "Add 3 agility"}}, 
							  {"Mana_save", {"Mana save", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "You'll get 2 more mana next turn"}}, 
							  {"Twice_act", {"Twice act", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "The next 'phy' card you use this turn will be processed twice"}}, 
							  {"Hypnotize", {"Hypnotize", "psy" , 2, 0, 0, 0, 0, 0,-1,-1, 0, 1, "Decrease a player's strength and defence for 1"}}
							 };
map<string, Card> cardDictPVP = {
							  {"Strike", {"Strike", "phy", 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"}}, 
							  {"Strikeee", {"Strikeee", "phy", 1, 0, 2, 0, 0, 0, 0, 0, 0, 3, "Deal 2 damage to a player 3 times"}}, 
							  {"Mana hit", {"Mana hit", "phy", 2, 0, 6, 0, 0, 0, 0, 0, 0, 1, "Deal 7 damage, and you'll get 1 more mana next turn"}}, 
							  {"Crash", {"Crash", "phy", 1, 0, 3, 0, 0, 1, 0, 0, 0, 1, "Deal 3 damage and draw 1 card"}}, 
							  {"Defend", {"Defend", "phy", 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"}}, 
							  {"Dodge", {"Dodge", "phy", 1, 1, 0, 4, 0, 2, 0, 0, 0, 1, "Get 4 block and draw 2 cards"}}, 
							  {"Power up", {"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, "Add 2 strength"}}, 
							  {"Armor up", {"Armor up" , "ablt", 1, 1, 0, 0, 0, 0, 0, 3, 0, 1, "Add 3 agility"}}, 
							  {"Mana save", {"Mana save", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "You'll get 2 more mana next turn"}}, 
							  {"Twice act", {"Twice act", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "The next 'phy' card you use this turn will be processed twice"}}, 
							  {"Hypnotize", {"Hypnotize", "psy" , 2, 0, 0, 0, 0, 0,-1,-1, 0, 1, "Decrease a player's strength and defence for 1"}}
							 };

// Player structure
struct Player {
	string name;
    int health, block, power, aglty;
	bool sheld;
    vector<Card> deck;
    vector<Card> hand;
	vector<Card> tomb;
	vector<Card> exil;
	vector<Card> stat;
} player, enemy;

// the initial health of PVE player and enemy
int PVE_PHealth = 60, PVE_EHealth = 80;
// the name of player and PVE enemy
string playerName = "player", PVE_enemyName = "zombie", enemyName;
// IP of pvp server
string serverIP = "127.0.0.1", serverPort = "5755"; // 5755 = stss = StS s = StS simulation
// for windows sockets
WSADATA wsaData;
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo *result = NULL, *ptr = NULL, hints;
int iResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

// Function to initialize player's deck
void deckImport(Player& player) {
	cout << "* Now cleaning up the old deck...\n";
	player.deck.erase(player.deck.begin(), player.deck.end());
	cout << "* Now starting to import deck from deck.txt\n";
	if(freopen("deck.txt", "r", stdin) == NULL){
		// no file for import then use startup deck
		freopen("CON", "r", stdin);
		cout<<"* No deck found. pls put a deck.txt under the folder of this program if importing. Now using the startup deck.\n";
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Strike"   , "phy" , 1, 0, 5, 0, 0, 0, 0, 0, 0, 1, "Deal 5 damage"});
		player.deck.push_back({"Mana hit" , "phy" , 2, 0, 6, 0, 0, 0, 0, 0, 0, 1, "Deal 7 damage, and you'll get 1 more mana next turn"});
		player.deck.push_back({"Strikeee" , "phy" , 1, 0, 2, 0, 0, 0, 0, 0, 0, 3, "Deal 2 damage to a player 3 times"});
		player.deck.push_back({"Crash"    , "phy" , 1, 0, 3, 0, 0, 1, 0, 0, 0, 1, "Deal 3 damage and draw 1 card"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Defend"   , "phy" , 1, 1, 0, 6, 0, 0, 0, 0, 0, 1, "Get 6 block"});
		player.deck.push_back({"Dodge"    , "phy" , 1, 1, 0, 4, 0, 2, 0, 0, 0, 1, "Get 4 block and draw 2 cards"});
		player.deck.push_back({"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, "Add 2 strength"});
		player.deck.push_back({"Power up" , "ablt", 1, 1, 0, 0, 0, 0, 2, 0, 0, 1, "Add 2 strength"});
		player.deck.push_back({"Armor up" , "ablt", 1, 1, 0, 0, 0, 0, 0, 3, 0, 1, "Add 3 agility"});
		player.deck.push_back({"Mana save", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "You'll get 2 more mana next turn"});
		player.deck.push_back({"Twice act", "mag" , 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, "The next 'phy' card you use this turn will be processed twice"});
		player.deck.push_back({"Hypnotize", "psy" , 2, 0, 0, 0, 0, 0,-1,-1, 0, 1, "Decrease a player's strength and defence for 1"});
	} else {
		// importing
		int deckSize;
		cin >> deckSize;
		string cardName;
		for (int i = 0; i < deckSize; i++){
			cin >> cardName;
			player.deck.push_back(cardDict[cardName]);
		}
		freopen("CON", "r", stdin);
	}
	cout << "* Deck loading complete.\n";
}

// Function to print the player's deck (player.deck)
void deckCheck(Player& player){
	for (unsigned int i = 0; i < player.deck.size(); i++)
		cout << player.deck[i].name << endl;
	return;
} 
// Function to show all cards involved by this program
void cardsShow(){
	cout << "All cards can be used are: " << endl << \
	"Strike		1	phy	Deal 5 damage" << endl <<\
	"Strikeee	1	phy	Deal 2 damage to a player 3 times" << endl <<\
	"Mana hit	2	phy	Deal 7 damage, and you'll get 1 more mana next turn" << endl <<\
	"Crash		1	phy	Deal 3 damage and draw 1 card" << endl <<\
	"Defend		1	phy	Get 6 block" << endl <<\
	"Dodge		1	phy	Get 4 block and draw 2 cards" << endl <<\
	"Power up	2	ablt	Add 2 strength" << endl <<\
	"Armor up	3	ablt	Add 3 agility" << endl <<\
	"Mana save	1	mag	You'll get 2 more mana next turn" << endl <<\
	"Twice act	1	mag	The next 'phy' card you use this turn will be processed twice" << endl <<\
	"Hypnotize	2	psy	Decrease a player's strength and defence for 1" << endl;
	return;
}

// Function to shuffle the deck
void shuffleDeck(Player& player){
	int cardNum = player.deck.size();
	for(int i = 0; i < cardNum; i++){
		srand(int(clock()*1000));
		std::vector<Card>::size_type select = rand() % (cardNum-i);
		player.deck.push_back(player.deck[select]);
		player.deck.erase(player.deck.begin() + select);
	}
}

// Function of drawing cards, shuffle GY into deck when no card in deck and dont draw with a full hand
void drawP(Player& player, unsigned int num){
	for(unsigned int i = 0; i < num; i++){
		if(player.deck.size() == 0){
			// doing as StS
			cout<<"\n* No cards in the deck, shuffling the GY into the deck...\n";
			player.deck = player.tomb;
			player.tomb.erase(player.tomb.begin(), player.tomb.end());
			shuffleDeck(player);
		} else if(player.hand.size() >= 10){
			// also doing as StS, not sending the card into the gy like HS :)
			cout<<"Your hand is too full!\n";
			return;
		}
		// get card into hand and delete it from the deck
	    player.hand.push_back(player.deck.back());
	    player.deck.pop_back();
	}
}

// Function to handle battling things
void battleP(Player& obj, int damage){
	obj.block -= damage;
	if (obj.block < 0) {
		obj.health += obj.block;
		obj.block = 0;
	}
	return;
}

// Function to simulate a duel between player and computer
void duelPVE(Player& player, Player& enemy) {
	// initialization of the duel
	player = {playerName, PVE_PHealth, 0, 0, 0, 0, player.deck, {}, {}, {}, {}};
	enemy = {PVE_enemyName, PVE_EHealth, 0, 0, 0, 0, {}, {}, {}, {}, {}};
	shuffleDeck(player);
	player.health = PVE_PHealth;
	enemy.health = PVE_EHealth;
	int mana = 3, savedMana = 0, drawNum = 5, Twice_act = 0;
	int enemyAction, enemyActionPara[3]; // enemyAction: 001: attack; 010: block; 100: power up 1;; MAX: 111 = 7
	
	while (player.health > 0 && enemy.health > 0) {
		// Decide the action of the enemy
		srand(int(clock()*1000));
		enemyAction = rand()%7 + 1;
		srand(int(clock()*1000));
		enemyActionPara[0] = rand()%4 + 6;
		srand(int(clock()*1000));
		enemyActionPara[1] = rand()%7 + 8;
		srand(int(clock()*1000));
		enemyActionPara[2] = rand()%1 + 1;
		
		// Player's turn
		cout << "\n- Player's turn:" << endl;
		drawP(player, drawNum);
		drawNum = 5;
		while(1){
			// Part II of the turn initialization: enemy action
			cout<<"! The "<< enemy.name << " is going to:\n";
        	int tmpEA = enemyAction;
			if(tmpEA%2) cout << "deal " << enemyActionPara[0] << " damage to you; ";
			tmpEA/=2;
			if(tmpEA%2) cout << "get " << enemyActionPara[1] << " block; ";
			tmpEA/=2;
			if(tmpEA%2) cout << "power up by " << enemyActionPara[2] << " points;";
			// Status outputing
			cout << "\n* Now, " << player.name << " has " << player.health << " heath, " << player.block << " block, " << player.power << " strength, and " << player.aglty << " agility;";
			cout << "\n while " << enemy.name << " has " << enemy.health << " heath, " << enemy.block << " block, " << enemy.power << " strength, and " << enemy.aglty << " agility;";
			cout << "\n* Size of your deck: " << player.deck.size() << " | grave yard: " << player.tomb.size() << " | exile: " << player.exil.size() << ";";
			cout << "\nWith " << mana << " mana left, choose a card to play: \n(  name		cost	type	content)\n0. End turn	/	/	/ \n";
			// hand display
			for (unsigned int i = 0; i < player.hand.size(); ++i)
				cout << i + 1 << ". " << player.hand[i].name << "	" << player.hand[i].cost << "	" << player.hand[i].type << "	" << player.hand[i].text << endl;
			unsigned int choice;
			cin >> choice;
			// card choice processing
			if (choice < 0 || choice > player.hand.size()) {
				// out of cards/controller (0)
				cout << "><Invalid choice!" << endl;
			} else if (choice == 0){
				// go to EP
				cout << "- Your turn ended.\n";
				break;
			} else if(player.hand[choice - 1].cost > mana) {
				cout << "><You don't have enough mana for that!\n";
			} else {
				// waiting the player for choosing the card to play
				Card playCard = player.hand[choice - 1];
				int objNum;
				if(playCard.obj == 0){
					cout << "? Play the card " << playCard.name << " to: (1 for yourself, 2 for the enemy)\n";
					while(1){
						cin >> objNum;
						if(objNum != 1 && objNum != 2){
							cout << "><Object not found, pls retry.\n";
						} else break;
					}
				} else objNum = playCard.obj;
				// the cost is always processed (and always processed first)
				mana -= playCard.cost;
				// processing card effects
				cout << "! You played " << playCard.name << endl;
				if (playCard.name == "Mana save"){ // Special processing cards
					savedMana += 2;
					cout<<"! 2 mana saved for next turn by 'Mana save'.\n";
				} else if (playCard.name == "Twice act"){
					Twice_act = 1;
				} else if (playCard.name == "Mana hit"){
					Player *object;
					if(objNum == 1) object = &player;
					else if(objNum == 2) object = &enemy;
					battleP(*object, max(playCard.damag + player.power, 0));
					cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
					savedMana += 1;
					cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
					if(Twice_act && playCard.type == "phy"){
						cout << "- Effect of 'Twice act':\n";
						battleP(*object, max(playCard.damag + player.power, 0));
						cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						savedMana += 1;
						cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
						Twice_act = 0;
					}
				} else {                           // usual cards whose effects are instant
					Player *object;
					if(objNum == 1) object = &player;
					else if(objNum == 2) object = &enemy;
					for(int i = 0; i < playCard.times; i++){
						if(playCard.damag != 0){
							battleP(*object, max(playCard.damag + player.power, 0));
							cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						}
						if(playCard.block != 0) object->block += max(playCard.block + object->aglty, 0);
						object->health += playCard.heal; // Not limited, the same as YGO and MTG
						if(playCard.draw != 0) drawP(player, playCard.draw);
						object->power += playCard.power;
						object->aglty += playCard.aglty;
						player.sheld = (player.sheld || playCard.sheld);
					}
					if(Twice_act){
						cout << "- Effect of 'Twice act':\n";
						// here i just copied the code upside
						for(int i = 0; i < playCard.times; i++){
							if(playCard.damag != 0){
								battleP(*object, max(playCard.damag + player.power, 0));
								cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
							}
							object->block += max(playCard.block + object->aglty, 0);
							object->health += playCard.heal;
							if(playCard.draw) drawP(player, playCard.draw);
							object->power += playCard.power;
							object->aglty += playCard.aglty;
							player.sheld = (player.sheld || playCard.sheld);
						}
						// till here, copied the code to process the card (again)
						Twice_act = 0;
					}
				}
				// used cards should be thrown into the gy except for ability cards which should be exiled
				if(playCard.type != "ablt") player.tomb.push_back(playCard);
				else player.exil.push_back(playCard);
				std::vector<Card>::size_type select = choice - 1;
				player.hand.erase(player.hand.begin() + select);
			}
			
			if (enemy.health <= 0) {
				cout << enemy.name << " is defeated!" << endl;
				return;
			}
		}
		
		// End turn: throw all cards in hand & clear enemy's block & 'Twice act' effect ends
		while(player.hand.size()){
			player.tomb.push_back(player.hand.back());
			player.hand.pop_back();
		}
		enemy.block = 0;
		Twice_act = 0;
		
		// Your Opponent's turn
		cout << endl << "- " << enemy.name << "'s turn:\n" << endl;
		Sleep(800);
		
		if(enemyAction%2){
			cout << "! " << enemy.name << " dealt " << max(enemyActionPara[0] + enemy.power, 0) << " damage to you.\n";
			battleP(player, max(enemyActionPara[0] + enemy.power, 0));
			cout << "* Now you have " << player.health << " health, and " << player.block <<" block;\n";
		}
		if (player.health <= 0) { // only need to check it here as no more damage dealt in the later actions of the enemy
			cout << "You were defeated by " << enemy.name << "!" << endl;
			break;
		} enemyAction/=2;
		if(enemyAction%2){
			cout << "! " << enemy.name << " gets " << max(enemyActionPara[1] + enemy.aglty, 0) << " block.\n";
			enemy.block += max(enemyActionPara[1] + enemy.aglty, 0);
			cout << "* Now " << enemy.name << " has " << enemy.health << " health and " << enemy.block << " block;\n";
		} enemyAction/=2;
		if(enemyAction%2){
			cout << "! " << enemy.name << " gets " << enemyActionPara[2] << " more strength and " << enemyActionPara[2] << " more agility.\n";
			enemy.power += enemyActionPara[2];
			enemy.aglty += enemyActionPara[2];
			cout << "* Now " << enemy.name << " has " << enemy.power << " strength and " << enemy.aglty << " agility.\n";
		}
		Sleep(3000);

		
		// End turn; Next turn
		player.block = 0; // BLOCK OVER
        mana = 3 + savedMana; // player mana reset
        savedMana = 0;
		cout << "\n- " << enemy.name << "'s turn ended;\n";
    }
    return;
}

// Function to simulate a duel between player and another player
void duelPVP(Player& player, Player& enemy) {
	// initialization of the duel
	if(winsockInit()) return;
	player = {playerName, 60, 0, 0, 0, 0, player.deck, {}, {}, {}, {}};
	enemy = {enemyName, 60, 0, 0, 0, 0, {}, {}, {}, {}, {}};
	shuffleDeck(player);
	int turnNum = 1;
	int mana = 3, savedMana = 0, drawNum = 5, Twice_act = 0;
	int E_mana = 3, E_savedMana = 0, E_drawNum = 5, E_Twice_act = 0;
	
	// TODO: Why not Stone Scissor Cloth
	bool first, FT = true; // Ft = First Turn (for the second player)
	if(recvString() == "Info: First"){
		cout << "\n- You go first.";
		first = true;
	} else{
		cout << "\n- You go second.";
		first = false;
	}
	
	while (true) {
		// Player's turn
		if(first || !FT){
			cout << "\n- " << player.name << "'s (your) turn " << turnNum << " :" << endl; // output except when secondhand and first turn
			drawP(player, drawNum);
			drawNum = 5;
		}
		while(1){
			if (!first && FT) break; // Actually not your turn! 
			
			// Status outputing
			cout << "\n* Now, you (" << player.name << ") has:\n   " << mana << " mana, " << player.health << " heath, " << player.block << " block, " << player.power << " strength, and " << player.aglty << " agility;";
			cout << "\nwhile your opponent (" << enemy.name << ") has:\n   " << E_mana << " mana, " << enemy.health << " heath, " << enemy.block << " block, " << enemy.power << " strength, and " << enemy.aglty << " agility;";
			cout << "\n* Size of your deck: " << player.deck.size() << " | grave yard: " << player.tomb.size() << " | exile: " << player.exil.size() << ";";
//			cout << "\n* Size of enemy deck: " << enemy.deck.size() << " | grave yard: " << enemy.tomb.size() << " | exile: " << enemy.exil.size() << ";";
			cout << "\nChoose a card in your hand to play: \n(  name		cost	type	content)\n0. End turn	/	/	/ \n";
			
			// hand display
			for (unsigned int i = 0; i < player.hand.size(); ++i)
				cout << i + 1 << ". " << player.hand[i].name << "	" << player.hand[i].cost << "	" << player.hand[i].type << "	" << player.hand[i].text << endl;
			unsigned int choice;
			cin >> choice;
			
			// card choice processing
			if (choice < 0 || choice > player.hand.size()) {
				// not of cards/controller (0)
				cout << "><Invalid choice!" << endl;
			} else if (choice == 0){
				// go to EP
				sendString("End Turn");
				cout << "- Your turn ended.\n";
				break;
			} else if(player.hand[choice - 1].cost > mana) {
				cout << "><You don't have enough mana for that!\n";
			} else {
				// waiting the player for choosing the card to play
				Card playCard = player.hand[choice - 1];
				sendString(playCard.name); // FIRST SENDING
				int objNum;
				if (playCard.obj == 0){
					cout << "? Play the card " << playCard.name << " to: (1 for yourself, 2 for the enemy)\n";
					while (1){
						cin >> objNum;
						if (objNum != 1 && objNum != 2){
							cout << "><Object not found, pls retry.\n";
						} else break;
					}
				} else objNum = playCard.obj;
				if (objNum == 1) sendString("1");       // SECOND-
				else if (objNum == 2) sendString("2");  // -SENDING
				// the cost is always processed (and always processed first)
				mana -= playCard.cost;
				// processing card effects
				cout << "! You played " << playCard.name << endl;
				if (playCard.name == "Mana save"){ // Special processing cards
					savedMana += 2;
					cout<<"! 2 mana saved for next turn by 'Mana save'.\n";
				} else if (playCard.name == "Twice act"){
					Twice_act = 1;
				} else if (playCard.name == "Mana hit"){
					Player *object;
					if (objNum == 1) object = &player;
					else if (objNum == 2) object = &enemy;
					battleP (*object, max(playCard.damag + player.power, 0));
					cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
					savedMana += 1;
					cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
					if (Twice_act){
						cout << "- Effect of 'Twice act':\n";
						battleP(*object, max(playCard.damag + player.power, 0));
						cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						savedMana += 1;
						cout<<"! 1 mana saved for next turn by 'Mana hit'.\n";
						Twice_act = 0;
					}
				} else {                           // usual cards whose effects are instant
					Player *object;
					if (objNum == 1) object = &player;
					else if (objNum == 2) object = &enemy;
					for (int i = 0; i < playCard.times; i++){
						if (playCard.damag != 0){
							battleP(*object, max(playCard.damag + player.power, 0));
							cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
						}
						if (playCard.block != 0) object->block += max(playCard.block + object->aglty, 0);
						object->health += playCard.heal; // Not limited, the same as YGO and MTG
						if (playCard.draw != 0) drawP(player, playCard.draw);
						object->power += playCard.power;
						object->aglty += playCard.aglty;
						player.sheld = (player.sheld || playCard.sheld);
					}
					if (Twice_act && playCard.type == "phy"){
						cout << "- Effect of 'Twice act':\n";
						// here i just copied the code upside
						for (int i = 0; i < playCard.times; i++){
							if (playCard.damag != 0){
								battleP(*object, max(playCard.damag + player.power, 0));
								cout << playCard.name << " dealt " << max(playCard.damag + player.power, 0) << " damage to " << object->name << ";" << endl;
							}
							object->block += max(playCard.block + object->aglty, 0);
							object->health += playCard.heal;
							if(playCard.draw) drawP(player, playCard.draw);
							object->power += playCard.power;
							object->aglty += playCard.aglty;
							player.sheld = (player.sheld || playCard.sheld);
						}
						// till here, copied the code to process the card (again)
						Twice_act = 0;
					}
				}
				// used cards should be thrown into the gy except for ability cards which should be exiled
				if (playCard.type != "ablt") player.tomb.push_back(playCard);
				else player.exil.push_back(playCard);
				std::vector<Card>::size_type select = choice - 1;
				player.hand.erase(player.hand.begin() + select);
			}
			
			if (player.health <= 0 && enemy.health <= 0) {
				cout << "Tie!" << endl;
				return;
			} if (player.health <= 0) {
				cout << player.name << " is defeated!" << endl;
				return;
			} if (enemy.health <= 0) {
				cout << enemy.name << " is defeated!" << endl;
				return;
			}
		}
		
		// End turn: throw all cards in hand & clear enemy's block & 'Twice act' effect ends
		while(player.hand.size()){
			player.tomb.push_back(player.hand.back());
			player.hand.pop_back();
		}
		enemy.block = 0; // enemy block over
        E_mana = 3 + E_savedMana; // enemy mana reset
		E_savedMana = 0;
		Twice_act = 0;
		if (!first && FT)
			FT = false;
		else if (!first)
			turnNum++;
		
		// opponent's turn
		cout << endl << "- " << enemy.name << "'s (opponent's) turn " << turnNum << " :\n" << endl;
		while (1) {
			cout << "\n* Now, you (" << player.name << ") has:\n   " << mana << " mana, " << player.health << " heath, " << player.block << " block, " << player.power << " strength, and " << player.aglty << " agility;";
			cout << "\nwhile your opponent (" << enemy.name << ") has:\n   " << E_mana << " mana, " << enemy.health << " heath, " << enemy.block << " block, " << enemy.power << " strength, and " << enemy.aglty << " agility;";
			cout << "\n* Size of your deck: " << player.deck.size() << " | grave yard: " << player.tomb.size() << " | exile: " << player.exil.size() << ";";
			if (player.hand.size() == 0) 
				cout << "\nYour hand is empty. \n";
			else {
				cout << "\nWhat's in your hand: \n(  name		cost	type	content)\n";
				// hand display
				for (unsigned int i = 0; i < player.hand.size(); ++i)
					cout << i + 1 << ". " << player.hand[i].name << "	" << player.hand[i].cost << "	" << player.hand[i].type << "	" << player.hand[i].text << endl;
			}
			
			// receive messages from server until turn ends
			string recStr = recvString();
			if (recStr == "Info: Card"){
				
				recStr = recvString();           // card name
				string objNumStr = recvString(); // objNum but string type
				
				Card playCard = cardDictPVP[recStr];
				E_mana -= playCard.cost;     // process cost
				
				// processing card effects
				cout << "\n! Your Opponent played " << playCard.name << endl;
				if (recStr == "Mana save"){ // Special processing cards
				E_savedMana += 2; 
					cout<<"! Your Opponent saved 2 mana for next turn by 'Mana save'.\n";
				} else if (recStr == "Twice act"){
					cout<<"! The next 'phy' card Your opponent used this turn will be processed twice.\n";
					E_Twice_act = 1;
				} else if (recStr == "Mana hit"){
					Player *object;
					if (objNumStr == "2") object = &player;
					else if (objNumStr == "1") object = &enemy;
					battleP (*object, max(playCard.damag + enemy.power, 0));
					cout << playCard.name << " dealt " << max(playCard.damag + enemy.power, 0) << " damage to " << object->name << ";" << endl;
					cout<<"! Your Opponent saved 1 mana for next turn by 'Mana hit'.\n";
					if (E_Twice_act){
						cout << "- Effect of 'Twice act':\n";
						battleP(*object, max(playCard.damag + player.power, 0));
						cout << playCard.name << " dealt " << max(playCard.damag + enemy.power, 0) << " damage to " << object->name << ";" << endl;
						E_savedMana += 1;
						cout<<"! Your Opponent saved 1 mana for next turn by 'Mana hit'.\n";
						E_Twice_act = 0;
					}
				} else {                           // usual cards whose effects are instant
					Player *object;
					if (objNumStr == "2") object = &player;
					else if (objNumStr == "1") object = &enemy;
					for (int i = 0; i < playCard.times; i++){
						if (playCard.damag != 0){
							battleP(*object, max(playCard.damag + enemy.power, 0));
							cout << playCard.name << " dealt " << max(playCard.damag + enemy.power, 0) << " damage to " << object->name << ";" << endl;
						}
						if (playCard.block != 0) object->block += max(playCard.block + object->aglty, 0);
						object->health += playCard.heal; // Not limited, the same as YGO and MTG
//						if (playCard.draw != 0) drawP(player, playCard.draw);
						object->power += playCard.power;
						object->aglty += playCard.aglty;
						enemy.sheld = (enemy.sheld || playCard.sheld);
					}
					if (E_Twice_act && playCard.type == "phy"){
						cout << "- Effect of 'Twice act':\n";
						// here i just copied the code upside
						for (int i = 0; i < playCard.times; i++){
							if (playCard.damag != 0){
								battleP(*object, max(playCard.damag + enemy.power, 0));
								cout << playCard.name << " dealt " << max(playCard.damag + enemy.power, 0) << " damage to " << object->name << ";" << endl;
							}
							object->block += max(playCard.block + object->aglty, 0);
							object->health += playCard.heal;
//							if(playCard.draw) drawP(player, playCard.draw);
							object->power += playCard.power;
							object->aglty += playCard.aglty;
							enemy.sheld = (enemy.sheld || playCard.sheld);
						}
						// till here, copied the code to process the card (again)
						E_Twice_act = 0;
					}
				}
			} else if (recStr == "Info: End turn"){
				break;
			} else if (recStr == "ERROR"){
				cout << "[ERROR ] Receiving invalid message from server, or connection with server closed.\n";
				return;
			} else if (recStr == "Info: Opponent left"){
				cout << "[ INFO ] Your opponent has left. You're the winner anyway! \n";
				return;
			}
			
			if (player.health <= 0 && enemy.health <= 0) {
				cout << "Tie!" << endl;
				return;
			} if (player.health <= 0) {
				cout << player.name << " is defeated!" << endl;
				return;
			} if (enemy.health <= 0) {
				cout << enemy.name << " is defeated!" << endl;
				return;
			}
		}

		
		// End turn; Next turn
		if (first) turnNum++;
		E_Twice_act = 0;
		player.block = 0; // BLOCK OVER
        mana = 3 + savedMana; // player mana reset
        savedMana = 0;
		cout << "\n- " << enemy.name << "'s (opponent's) turn ended;\n";
    }
    
    // close socket
    closesocket(ConnectSocket);
    cout << "[ info ] Server disconnected. \n";
    
    cout << "\nPress any key to continue...\n";
    getch();
    
    return;
}

int main() {
	init();
	cout << "Welcome to the wsfStS! \n* Automatically importing the deck...\n";

	deckImport(player);

	string input;
	while(1){
		// main menu 
		cout << "\nPls select a mode to play(input the number):\n";
		cout << "1. PVE mode\n2. PVP mode\n";
		cout << "3. settings\n4. about\n5. exit the game\n";
		cin >> input;
		if (input == "1"){
			// single player
			duelPVE(player, enemy);
			player.health = PVE_PHealth;
			enemy.health = PVE_EHealth;
			cout << "PVE mode game over!\n";
		} else if (input == "2"){
			cout << "! WHAT YOU NEED TO KNOW ABOUT PVP MODE: \n" \
			     << "1.You can set the server and player name in settings;\n" \
			     << "2.PVP mode is still in progress, thus it may have bugs.\n";
			
			duelPVP(player, enemy);
			player.health = PVE_PHealth;
			enemy.health = PVE_EHealth;
			cout << "PVP mode game over!\n";
		} else if (input == "3"){
			// settings menu
			while (1) {
				cout << "\nSettings:\n";
				cout << "0. Quit Settings\n";
				cout << "1. Change player name (" << playerName << ")\n";
				cout << "2. Set PVE mode enemy name (" << PVE_enemyName << ")\n";
				cout << "3. Set PVE mode player health (" << PVE_PHealth << ")\n";
				cout << "4. Set PVE mode enemy health (" << PVE_EHealth << ")\n";
				cout << "5. Set PVP mode server IP (" << serverIP << ")\n";
				cout << "6. Set PVP mode server port (" << serverPort << ")\n";
				cout << "7. Check your deck\n";
				cout << "8. Import a deck\n";
				cout << "9. Show all cards\n";
				cin >> input;
				if (input == "0"){
					break;
				} else if (input == "1"){
					cout << "Input the new player name: ";
					cin >> playerName;
				} else if (input == "2"){
					cout << "Input the new enemy name: ";
					cin >> PVE_enemyName;
				} else if (input == "3"){
					cout << "Input the new player health: ";
					cin >> PVE_PHealth;
				} else if (input == "4"){
					cout << "Input the new enemy health: ";
					cin >> PVE_EHealth;
				} else if (input == "5"){
					cout << "Input the new server IP: ";
					cin >> serverIP;
				} else if (input == "6"){
					cout << "Input the new server port: ";
					cin >> serverPort;
				} else if (input == "7"){
					deckCheck(player);
				} else if (input == "8"){
					deckImport(player);
				} else if (input == "9"){
					cardsShow();
				} else {
					cout << "* Invalid choice, pls try again.\n";
				}
			}
		} else if (input == "4"){
			cout << "A small simulate of StS but of CLI.\n";
			cout << "Made by CVE-2020-0796 on 18th May, 2024.\n";
			cout << "bilibili:  https://space.bilibili.com/518624115\n";
			cout << "YouTube:  https://www.youtube.com/channel/UCazOP122G3Yo0JXFjuAfTlQ\n";
			cout << "website:  https://wfs-2022.github.io/myNewWebsite/\n";
		} else if (input == "5") break;
		else cout << "* Invalid choice, pls try again.\n";
	}
	return 0;
}

void init(){
	system("title wsfStS");
//	CONSOLE_CURSOR_INFO cursor_info = {1, 0};
//	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
	char cmd[64];
	sprintf(cmd, "mode con cols=%d lines=%d", 60, 40);
//	system(cmd);
//	SetWindowLongPtrA(GetConsoleWindow(), GWL_STYLE, GetWindowLongPtrA(GetConsoleWindow(),GWL_STYLE)& ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
}
int winsockInit(){
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cerr << "[ERROR ] WSAStartup failed: " << iResult << endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(serverIP.c_str(), serverPort.c_str(), &hints, &result);
    if (iResult != 0) {
        cerr << "[ERROR ] getaddrinfo failed: " << iResult << endl;
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            cerr << "[ERROR ] socket failed: " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        // Connect to server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cerr << "[ERROR ] Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }
    
    // Send the playername to the server
    int iResult = send(ConnectSocket, playerName.c_str(), playerName.length(), 0);
    if (iResult == SOCKET_ERROR) {
        cerr << "[ERROR ] Failed to send the playername to the server " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        return 1;
    }
    
	if (recvString() == "Error: Server is full. "){
		cerr << "[ERROR ] The Server is full\n";
		return 1;
	}
    
	cout << "[ INFO ] Server " << serverIP << ":" << serverPort << " connected. \n";
	
	if (recvString() == "Wait: Waiting for opponent. "){
		cout << "[ INFO ] Now waiting for an opponent to start...\n";
		enemyName = recvString();
	} else enemyName = recvString();
    
    return 0;
}
void sendString(string sendString){
    // Send a string to the server
    int iResult = send(ConnectSocket, sendString.c_str(), sendString.length(), 0);
    if (iResult == SOCKET_ERROR) {
        cerr << "[ERROR ] Failed to send to server: " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
		return;
    }
	return;
} string recvString(){
    // Receive a string from the server, return the string
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        string receivedString(recvbuf, iResult);
//        cout << "[ INFO ] Received from server: " << receivedString << endl;
		return receivedString;
    } else if (iResult == 0)
        cout << "[ INFO ] Connection with server closing...\n";
    else
        cerr << "[ERROR ] Receiving from server failed: " << WSAGetLastError() << endl;
    return "ERROR";
}

//POINT GetMousePos(){
//	POINT p;
//	GetCursorPos(&p);
//	ScreenToClient(GetForegroundWindow(),&p);
//	CONSOLE_FONT_INFO consoleCurrentFont;
//	GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &consoleCurrentFont);
//	p.x/=consoleCurrentFont.dwFontSize.X;
//	p.y/=consoleCurrentFont.dwFontSize.Y;
//	return p;
//}
//void gotoxy(int x, int y){
//	HANDLE hout;
//	COORD pos;
//	pos.X = x;
//	pos.Y = y;
//	hout = GetStdHandle(STD_OUTPUT_HANDLE);
//	SetConsoleCursorPosition(hout, pos);
//}
