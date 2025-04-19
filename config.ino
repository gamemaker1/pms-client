typedef struct Slot {
  String location;
  long detected; bool parked;
  int trig; int echo;
  int gled; int rled;
} Slot;

Slot slots[] = {
  { "A1", -1, false, 14, 27, 25, 26 },
  { "A2", -1, false, 33, 32, 2, 4 },
};
int SLOT_COUNT = sizeof(slots) / sizeof(Slot);

String ssid = "Baarish";
String password = "inthiseconomy?";
String secret = "5cd224715682a288017719300dc93b6d";
String serverUrl = "http://192.168.12.1:4242/api";
