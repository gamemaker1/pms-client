typedef struct Slot {
  String location;
  long detected;
  bool parked;
  int trig; int echo;
  int gled; int rled;
  int threshold;
} Slot;

Slot slots[] = {
  { "A1", -1, false, 14, 27, 26, 25, 3 },
  { "A2", -1, false, 32, 33, 2, 4, 4 },
  { "A3", -1, false, 17, 18, 15, 16, 3 },
  // { "B1", -1, false, 14, 27, 26, 25, 4 },
  // { "B2", -1, false, 32, 33, 2, 4, 3 },
  // { "B3", -1, false, 17, 18, 15, 16, 3 },
};
int SLOT_COUNT = sizeof(slots) / sizeof(Slot);

String ssid = "Baarish";
String password = "inthiseconomy?";
String secret = "5cd224715682a288017719300dc93b6d";
String serverUrl = "http://192.168.12.1:4242/api";
