#pragma once
constexpr int MAX_BUFFER = 1024;
constexpr short SERVER_PORT = 3500;
constexpr int MAX_NAME = 50;

constexpr int BOARD_WIDTH = 8;
constexpr int BOARD_HEIGHT = 8;

constexpr unsigned char C2S_PACKET_LOGIN =		 1;
constexpr unsigned char C2S_PACKET_MOVE =		 2;

constexpr unsigned char S2C_PACKET_LOGIN_INFO =	 3;
constexpr unsigned char S2C_PACKET_PC_LOGIN =	 4;
constexpr unsigned char S2C_PACKET_PC_MOVE =	 5;
constexpr unsigned char S2C_PACKET_PC_LOGOUT =	 6;

#pragma pack (push, 1)
struct c2s_packet_login
{
	unsigned char size;
	unsigned char type;
	char name[MAX_NAME];
};

struct c2s_packet_move
{
	unsigned char size;
	unsigned char type;
	char dir; // 0�� 1���� 2�Ʒ� 3��
};

struct s2c_packet_login_info
{
	unsigned char size;
	unsigned char type;
	int id;
	short x, y, hp, level;
};

struct s2c_packet_pc_login
{
	unsigned char size;
	unsigned char type;
	int id;
	char name[MAX_NAME];
	short x, y;
	char o_type;
};

struct s2c_packet_pc_move
{
	unsigned char size;
	unsigned char type;
	int id;
	short x, y;
};

struct s2c_packet_pc_logout
{
	unsigned char size;
	unsigned char type;
	int id;
};
#pragma pack (pop)