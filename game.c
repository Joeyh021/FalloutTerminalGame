#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
//written for 4 letter words only for now, *may& change in future
//also written for a fixed terminal size of 80x24, this may change in future too


struct coord{ //struct to represent coordinates of cursor on screen
    int y;
    int x;
};

void output_text(WINDOW* output,char text[]){//output messages to terminal window in console
    static int y = 1;
    mvwprintw(output,++y,1,">>> %s", text);
    wrefresh(output);
}

void generate_text(WINDOW *textwin){//generates the random text and adds it to screen
    int line_len = 26;
    int n_lines = 30;
    int i,j;
    char l;
    int hex_val = (rand() % ((0x10000000) - (0x100000))) + (0x100000); //generate the hex values, range used for aesthetics

    for (i=1;i<n_lines;i+=2){
        mvwprintw(textwin,i,1,"%#010X  ",hex_val+i*4);
        for(j=1;j<line_len;j++){
            l = (rand() % (126 - 33 + 1)) + 33;
            mvwaddch(textwin,i,j+12,l);
        }//first column
    }
    for (i=1;i<n_lines;i+=2){
        mvwprintw(textwin,i,42,"%#010X  ",hex_val+i*4);
        for(j=1;j<line_len;j++){
            l = (rand() % (126 - 33 + 1)) + 33;
            mvwaddch(textwin,i,j+53,l);
        }//second column
    }
    wrefresh(textwin);
}

//determine how similar two strings are by iterating through, compare each char
int check_similarity(int wordlen, char worda[wordlen+1],char wordb[wordlen+1]){
    int similarity = 0;
    int i;
    for (i=0; i<wordlen; i++){
        //printf("%c %c \n", worda[i],wordb[i]);
        if(worda[i] == wordb[i]){
            similarity ++;
        }
    }
    return similarity;
}

void load_words(FILE* fp,int wordlen, int no_of_words, char wordarr[no_of_words][wordlen+1]){
    //create an array of numbers detailing how many words of each similarity to include
    int sim_dist[no_of_words];
    sim_dist[0] = wordlen;
    int i;

    for(i=0; i<no_of_words; i++){
        sim_dist[i+1] = wordlen - (rand() % (i+2) + 1);
        if (sim_dist[i+1] < 0){
            sim_dist[i+1] = 0;
        }
    }
    //print similarity distributon if needed - for debugging
    /*for(i=0; i<no_of_words; i++){
        printf("word %d has sim %d \n",i,sim_dist[i]);
    }*/ 

    //
    char buffer[wordlen+1];
    int count = 0;

    while (count != no_of_words){

        if(fgets(buffer,wordlen+2,fp) == NULL){
            fseek(fp,0,SEEK_SET); //if EOF jump back to top of file

        }else if(rand() % 100 == 5){
            buffer[wordlen] = '\0'; //set last char of buffer to null terminator 

            if(count == 0){
                strcpy(wordarr[count],buffer); //on first iteration use as target word, dont check similarity, copy into arr
                count++;

            }else if(check_similarity(wordlen,buffer,wordarr[0]) == sim_dist[count]){
                strcpy(wordarr[count],buffer); //check if chosen word fits similarity reqiurements, copy into arr if it does
                count++;
            }
        }
    }

}

void generate_positions(int n_words, struct coord pos[n_words]){
    int i,line_no; //create array to hold positions of each word on screeen
    for(i=0; i<n_words;i++){
        line_no = (rand() % 30)+ 1;
        if (line_no >= 15){
            pos[i].y = ((line_no-15)*2)-1;
            pos[i].x = (rand() % 19) + 54;
        }else{
            pos[i].y = (line_no*2)-1;
            pos[i].x = (rand() % 20) + 14;
        } //char number
    }
}
void insert_words(WINDOW* textwin, int n_words, int wordlen, struct coord pos[n_words],char wordarr[n_words][wordlen+1],WINDOW* out){
    int line_no,char_no,i; //insert the words onto the screen amongst the text and the generated positions
     for(i=0; i<n_words;i++){
        line_no = pos[i].y;
        char_no = pos[i].x; 
        mvwprintw(textwin,line_no,char_no,wordarr[i]);
        /*output_text(out,wordarr[i]);
        printf("%s line %d char %d \n", wordarr[i],pos[i].y,pos[i].x);*/
        
    }
}
void clear_highlight(WINDOW* textwin){
    int i; //remove all cursor highlighting from screen
    for(i=1;i<31;i++){
        mvwchgat(textwin,i,1,80,A_NORMAL,1,NULL);
    }
}

int check_pos(int n_words,struct coord pos[n_words],struct coord current_pos){
    int i; //check a cursor position to see if it is in on a word
    int word = -1;
    for(i=0; i<n_words;i++){
        if (pos[i].y == current_pos.y){
            if ((pos[i].x <= current_pos.x) && (current_pos.x <= (pos[i].x + 3))){//words are 4 letters long, check range
                word = i;
            }
        }
    }
    return word;
}

struct coord move_cursor(WINDOW* textwin,struct coord old,int dir){
    struct coord new = old; //move the cursor
    switch(dir){
        case 1: //1 -  right 2 - up 3 - left 4 - down
            new.x +=1;
            break;
        case 2:
            new.y += 2;
            break;
        case 3:
            new.x -=1;
            break;
        case 4:
            new.y -=2;
            break;
    }
    if ((new.y > 29) || (new.y < 1)){//dont go off screen
        new.y = old.y;
    }
    if ((new.x > 78) || (new.x < 13)){//also dont go off screeen
        new.x = old.x;
    }
    if (new.x == 38){
        new.x = 54;
    }
    if (new.x == 53){//jump accross middle gap
        new.x = 37;
    }

    clear_highlight(textwin);
    mvwchgat(textwin,new.y,new.x,1,A_STANDOUT,2,NULL);
    wrefresh(textwin);
    return new;

}
int main(int argc, char*argv[]){
    
    WINDOW *header;
    WINDOW *text;
    WINDOW *output; //the three windows that make up the game

    initscr();
    clear();
    cbreak();
    curs_set(0);
    keypad(stdscr,true);
    noecho();
    start_color();
    init_pair(1,COLOR_GREEN,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_WHITE); //initialise stuff
    attron(COLOR_PAIR(1));
    srand(time(0));
    
    /*
    int max_rows,max_cols;
    getmaxyx(stdscr,max_rows,max_cols);
    
    if ((max_rows < 80) || (max_cols < 24)){
        mvprintw(0,0,"screen too small! your terminal is %dx%d. 80x24 min required \n", max_cols,max_rows);
        sleep(1);
        endwin();
        exit(1);
    }
    */
    refresh();

    header = newwin(11,82,0,0);
    wattron(header,COLOR_PAIR(1));
    mvwprintw(header,1,1,"ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL");
    mvwprintw(header,3,1,"ENTER PASSWORD NOW");
    mvwprintw(header,5,1,"ATTEMPTS LEFT - 3");
    mvwprintw(header,8,1,"PRESS Q TO QUIT");
    box(header,0,0);
    wrefresh(header);

    output = newwin(43,50,0,82);
    wattron(output,COLOR_PAIR(1));
    box(output,0,0);
    wrefresh(output);

    text = newwin(32,82,11,0);
    wattron(text,COLOR_PAIR(1));
    box(text,0,0);
    wrefresh(text);

    generate_text(text);
    refresh();//setup the three windows and generate the text 

    FILE *wordlist = fopen("4-wordlist.txt","r");
    int wordlen = 4;
    int n_words = (rand() % 4) + 6;
    char wordarr[n_words][wordlen+1];
    load_words(wordlist,wordlen,n_words,wordarr); //seup wordlist and load rrandom words from file
    /*for(int i=0; i<n_words;i++){
        output_text(output,wordarr[i]);
    }*/

    struct coord pos[n_words];

    generate_positions(n_words,pos);
    insert_words(text,n_words,wordlen,pos,wordarr,output);
    wrefresh(text);//generate the word positions and insert them

    struct coord cursor_pos;
    cursor_pos.y=1;
    cursor_pos.x=13;

    mvwchgat(text,cursor_pos.y,cursor_pos.x,1,A_STANDOUT,2,NULL);
    wrefresh(text);
    check_pos(n_words,pos,cursor_pos);
    refresh();//draw cursor

    int ch,current_word,attempts;
    attempts = 3;

    while((ch = getch()) != 'q'){
        switch (ch){
            case KEY_LEFT:
                cursor_pos = move_cursor(text,cursor_pos,3);
                break;
            case KEY_RIGHT:
                cursor_pos = move_cursor(text,cursor_pos,1);
                break;
            case KEY_UP:
                cursor_pos = move_cursor(text,cursor_pos,4);
                break;
            case KEY_DOWN:
                cursor_pos = move_cursor(text,cursor_pos,2);//movement
                break;
            case 10: //enter key pressed
                if (current_word != -1){
                    char word[5];
                    strcpy(word,wordarr[current_word]);
                    attempts-=1;
                    int sim = check_similarity(4,wordarr[0],word);
                    mvwprintw(header,5,17,"%d",attempts);
                    wrefresh(header);
                    int i;
                    for (i=0;i<4;i++){
                        word[i] -=32;
                    }
                    output_text(output,word);
                    if(sim == 4){
                        output_text(output,"PASSWORD ACCEPTED - ACCESS GRANTED");
                        sleep(2);
                        endwin();
                        exit(1);
                    }else{
                        output_text(output,"ENTRY DENIED");
                        char message[15] = "LIKENESS - ";
                        char likeness[2] = {sim + 48};
                        strcat(message,likeness);
                        output_text(output,message);
                    }
                    if (attempts <= 0){
                        output_text(output,"ALL ATTEMPTS USED - LOCKING TERMINAL");
                        sleep(1);
                        output_text(output,"QUITTING ...");
                        sleep(1);
                        endwin();
                        exit(1);
                    }
                }
                break;
        }

        current_word = check_pos(n_words,pos,cursor_pos);

        if (current_word != -1){
                mvwchgat(text,pos[current_word].y,pos[current_word].x,4,A_STANDOUT,2,NULL);
                if(ch == KEY_LEFT){
                    cursor_pos.x -=3;
                }
                if (ch == KEY_RIGHT){
                    cursor_pos.x +=3;
                }
        }

    wrefresh(text);
    refresh();
    }
    output_text(output,"QUITTING ...");
    sleep(1);
    endwin();
    exit(1);
    return 0;
}
