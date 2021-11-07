String processor(const String& var) {
      if(var == "TEMP") return String(luftTemp);
      if(var == "PRESS") return String(redLuftDruck);
      if(var == "HUMI") return String(luftFeuchte);
      if(var == "DEW") return String(luftDew);
      if(var == "TIME") return String(akttime);
      if((var == "TOK") & (timeok = true)) return "OK";
      if((var == "TOK") & (timeok = false)) return "NOT OK";
      return String();
    }
    
