#include <Arduino.h>
#include <LiquidCrystal.h>
#include "Process.h"
#include "LCDisplay.h"

LCDisplay::LCDisplay(int rsVal, int enVal, int d4Val, int d5Val, int d6Val, int d7Val) {
    rs = rsVal;
    en = enVal;
    d4 = d4Val;
    d5 = d5Val;
    d6 = d6Val;
    d7 = d7Val;
    interval = 1000; // half second update
}

void LCDisplay::setup() { 
    LiquidCrystal l1(rs, en, d4, d5, d6, d7 );
    lcd = &l1;
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    delay(1);
    lcd->begin(16,2);
    lcd->clear();
    lcd->setCursor(0,0);
    lcd->print("Hello world");
    strcpy(line0, "??.?C hh:mm ???%");
    strcpy(line1, "                ");
    strcpy(newsItems[0], "Testing one two three four"); // getHeadline();
    // newsItems[1] = getHeadline();
    // newsItems[2] = getHeadline();
    // newsItems[3] = getHeadline();
    // newsItems[4] = getHeadline();
}

void LCDisplay::action() {
    updateClock();    
    if (this->state == LOW) {
        this->state = HIGH;
        this->interval = 500;
    } else {
        this->state = LOW;
        this->interval = 500;
    }
    digitalWrite(13, this->state);
    // scrollNews();
    // updateDisplay();
}

void LCDisplay::useClock(RTClock *clockPtr) {
    clock = clockPtr;
}

void LCDisplay::scrollNews() {
        unsigned int tempItem;  
        unsigned int tempPos; 

        tempItem = curItem; 
        tempPos = curPos; 

        // fill buffer from current position
        for (int i = 0; i < 16; i++)
        {
            line1[i] = newsItems[tempItem][tempPos++];
            if (newsItems[tempItem][tempPos] == '\0') {
                tempPos = 0;
                // inject separators
                if (i < 16) { line1[i++] = ' '; }
                if (i < 16) { line1[i++] = '*'; }
                if (i < 16) { line1[i++] = ' '; }
             //   if (++tempItem >= LCDisplay::numItems) {
             //       tempItem = 0;
             //   }
            }
        }
        // update current position
 /*       if (++curPos >= newsItems[curItem].length()) {
            curPos = 0; */
       /*     if (++curItem >= LCDisplay::numItems) {
                curItem = 0;
                // shuffle items down
                for (int j = 0; j < LCDisplay::numItems - 1; j++)
                {
                    newsItems[j] = newsItems[j+1];
                }
                newsItems[LCDisplay::numItems] = getHeadline();
            } 
        } */
    }


void LCDisplay::updateDisplay() {
    lcd->setCursor(0,0);
    lcd->print(line0);
    lcd->print(line1);
}

void LCDisplay::updateClock() {
    char temp[3];
    int hh;
    int mm;

    lcd->setCursor(0,1);
    if (blink) {
        line0[9] = ' ';
        blink = false;
        lcd->print("*");
    } else {
        line0[9] = ':';
        blink = true;
        lcd->print("+");
    }
    // lcd->setCursor(2,1);
    // lcd->print(millis());
    if (++clockCounter > 58) {
        clockCounter = 0;
        hh = clock->getHours();
        itoa(hh, temp, 10);
        if (hh < 10) {
            line0[7] = '0';
            line0[8] = temp[0];
        } else {
            line0[7] = temp[0];
            line0[8] = temp[1];
        }
        mm = clock->getMinutes();
        itoa(mm,temp, 10);
        if (mm < 10) {
            line0[10] = '0';
            line0[11] = temp[0];
        } else {
            line0[10] = temp[0];
            line0[11] = temp[1];
        }
    }
}

    
char *LCDisplay::getHeadline() {       
    static const char* const subjects1[] = { "cat", "dog", "man", "alien", "sheep", "mayor",
                    "boy", "girl", "ship", "train", "car", "fish",
                    "emmet" };
    static const char* const subjects2[] = { "we are all", "you are", "entire world", 
                "England team", "Britney Spears", "Chief of Police", "the last lot",
                "The Beatles", "I am"};
    static const char* const verbs1[] = { "stuck in", "climbs", "falls of", "finds", 
                "loses", "steals", "given",
                "eats", "buys", "crashes in to", "sells" };
    static const char* const verbs2[] = { "in short supply", "almost gone", "falls from sky", 
                "explodes", "gives up",
                "stolen again", "painted green", "spotted", "left behind" };
    static const char* const objects1[] = { "cupcake", "tree", "flying saucer", "red jelly",
                "ladder", "pizza van",
                "fountain", "fire engine", "police car", "coffee cup", "trousers",
                "lamp post", "park bench", "seagull", "dartboard" };
    static const char* const adjectives1[] = { "mad", "wrong", "pink", "smelly", "hopeless", 
                "loved", "great",
                "too fat", "vegan", "awesome", "happy", "tiny", "over-rated", "stupid" };
    static const char* const complete[] = { "Go sports team!", "AFOLs are Cool", 
                "Everything is awesome", "Lego City News Service"};

    headline[0] = '\0';

    int percent = random(1,101);  // get a percentage
    if (percent < 30) {
        strcpy(headline, subjects1[random(0,sizeof(subjects1) / sizeof(subjects1[0]))]);
        strcat(headline," ");
        strcat(headline,verbs1[random(0,sizeof(verbs1) / sizeof(verbs1[0]))]);
        strcat(headline," ");
        strcat(headline,objects1[random(0,sizeof(objects1) / sizeof(objects1[0]))]);
    } else if (percent < 60) {
        strcpy(headline,"\"");
        strcat(headline,subjects2[random(0,sizeof(subjects2) / sizeof(subjects2[0]))]);
        strcat(headline," ");
        strcat(headline,adjectives1[random(0,sizeof(adjectives1) / sizeof(adjectives1[0]))]);
        strcat(headline,"\" says ");
        strcat(headline,subjects1[random(0,sizeof(subjects1) / sizeof(subjects1[0]))]);
    } else if (percent < 90) {
        strcpy(headline,objects1[random(0,sizeof(objects1) / sizeof(objects1[0]))]);
        strcat(headline," ");
        strcat(headline,verbs2[random(0,sizeof(verbs2) / sizeof(verbs2[0]))]);
        if (percent > 70) {
            strcat(headline," claims ");
            if (percent > 80) {
                strcat(headline,adjectives1[random(0,sizeof(adjectives1) / sizeof(adjectives1[0]))]);
            strcat(headline," ");
            }
            strcat(headline,subjects1[random(0,sizeof(subjects1) / sizeof(subjects1[0]))]);
        }
    } else {
        strcat(headline, complete[random(0,sizeof(complete) / sizeof(complete[0]))]);
    }
    int pos = 0;
    char initial = headline[pos];
    if (initial == '"') {
        initial = headline[++pos];
    }
    headline[pos] = toupper(initial);
    return headline;
}