String JsonRelayString(char *zone, char *addr, char *value, char *remark ){
    String JsonString; 
    JsonString = "{\"Z\":\"";
    JsonString += zone;
    JsonString += "\",";
    JsonString += "\"S\":\"";
    JsonString += addr;  
    JsonString += "\",";
    JsonString += "\"V\":\"";
    JsonString += value;
    JsonString += "\",";
    JsonString += "\"R\":\"";
    JsonString += remark;
    JsonString += "\"}";

    return(JsonString);
}
