#pragma once

void Network_Handle_KidPosition(SLNet::Packet *p);
void Network_Send_KidPosition(unsigned char frame, unsigned char x, unsigned char y, char direction, unsigned char room, char alive, unsigned short curr_seq, unsigned char sword, unsigned short current_level, char curr_col, char curr_row, unsigned char action, char fall_x, char fall_y, unsigned char repeat, unsigned char charid);
void Network_Handle_UpdatePlayerColor(SLNet::Packet *p);
void Network_Send_UpdatePlayerColor(unsigned long long networkId, unsigned char red, unsigned char green, unsigned char blue);