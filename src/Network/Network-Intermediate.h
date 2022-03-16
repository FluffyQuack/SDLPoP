#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define NETWORK_MAXCLIENTS 128

void Network_Intermediate_Start(int networkStart, int port, char *ip, char *name, unsigned char colourRed, unsigned char colourGreen, unsigned char colourBlue);
void Network_Update();
void Network_Intermediate_Stop();
void Network_Intermediate_SendPrinceData(unsigned char frame, unsigned char x, unsigned char y, char direction, unsigned char room, char alive, unsigned short curr_seq, unsigned char sword, unsigned short current_level, char curr_col, char curr_row, unsigned char action, char fall_x, char fall_y, unsigned char repeat, unsigned char charid);
bool Network_Intermediate_GetPrinceData(int playerNum, unsigned char *frame, unsigned char *x, unsigned char *y, char *direction, unsigned char *room, char *alive, unsigned short *curr_seq, unsigned char *sword, unsigned short *current_level, char *curr_col, char *curr_row, unsigned char *action, char *fall_x, char *fall_y, unsigned char *repeat, unsigned char *charid);
void Network_Intermediate_GetPlayerColours(int playerNum, unsigned char *r, unsigned char *g, unsigned char *b);
char *Network_Intermediate_GetPlayerName(int playerNum);

#ifdef __cplusplus
}
#endif