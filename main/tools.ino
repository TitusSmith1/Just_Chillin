/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

String toArrayString(String arr[]) {
    String result = "["; // Start with the opening bracket
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result += "\"" + String(arr[i]) + "\""; // Add each element wrapped in quotes
        if (i < ARRAY_SIZE - 1) {
            result += ", "; // Add a comma and space between elements
        }
    }
    result += "]"; // Close with the closing bracket
    return result;
}

String toArrayString(float arr[]) {
    String result = "["; // Start with the opening bracket
    for (int i = 0; i < ARRAY_SIZE; i++) {
        result += "\"" + String(arr[i]) + "\""; // Add each element wrapped in quotes
        if (i < ARRAY_SIZE - 1) {
            result += ", "; // Add a comma and space between elements
        }
    }
    result += "]"; // Close with the closing bracket
    return result;
}
