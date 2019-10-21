/*
  Home Cockpit Solutions
  Software Arts 2019
  
  
*/

#include <Ethernet.h>


/*Ethernet Related*/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte iocpServer[] = {192,168,1,38};
int iocpPort = 8092;
EthernetClient  client;
int eth_rd_idx = 0;
int eth_rx_len = 0;     //gelen paketin genisligi
byte eth_rx_buf[1428];  //gelen paket
static byte tout = 0;

#define MAX_DEGISKEN    35

int val_list[MAX_DEGISKEN][2] = {{10,0},{11,0},{12,0},{13,0},{15,0},{16,0},{17,0},{18,0},{19,0},{20,0},{22,0},{23,0},{24,0},{25,0},{26,0},{27,0},{28,0},{29,0},{30,0},{32,0},{33,0},{34,0},{35,0},{36,0},{37,0},{40,0},{41,0},{45,0},{47,0},{48,0},{49,0},{53,0},{54,0},{55,0},{56,0}};
 
void parse_response(void);
 

void init_IO(void)
{
    for (int i=0; i<MAX_DEGISKEN; i++)
    {
      pinMode(i, OUTPUT);
    }  

    Serial.println("IO inited");
}

void init_UART(void)
{
    Serial.begin(9600);
    Serial.setTimeout(5);

    Serial.println("UART inited");
}

void init_ETHERNET(void)
{
    Ethernet.begin(mac);
    client.setTimeout(5);
    delay(1000);

    Serial.println("ETHERNET inited");
}

void setup() {

  init_IO();
  init_UART();
  init_ETHERNET();

  
}

void connect_iocp(void)
{
  int con_tout = 0;
  
  Serial.println("Connecting IOCP");
  
  client.connect(iocpServer, iocpPort); 
  while ((!(client.connected())) && (con_tout < 100))   //baglanana kadar 1 saniye boyunca bekle.
  {
    delay(10);
    Serial.print(".");

    con_tout++;
  }
  Serial.println(" ");

  if (client.connected())
  {
    Serial.println("Connected :)");
  }
  else
  {
    Serial.println("IOCP not responding :(");
  }
}

void send_registers(void)
{
  if (client.connected())
  {
    client.print("Arn.Preg:10:11:12:13:15:16:17:18:19:20:22:23:24:25:26:27:28:29:30:32:33:34:35:36:37:40:41:45:47:48:49:53:54:55:56\r\n");
    
    delay(10);
    
    client.print("Arn.Inicio:10:11:12:13:15:16:17:18:19:20:22:23:24:25:26:27:28:29:30:32:33:34:35:36:37:40:41:45:47:48:49:53:54:55:56\r\n");
  }
  else {while(1);}
}

void check_response(void)
{
    byte r_buf[2] = {0};
    //String gelen_paket;

    while (1)
    {
      if (client.available() > 0)
      {        
        client.readBytes(r_buf, 1);
        eth_rx_buf[eth_rd_idx] = r_buf[0];
        r_buf[0] = 0; tout = 0;
        
        eth_rd_idx++;
      }
      else 
      {
        tout++;
        delay(1);   //100milisaniye bir sey gelmezse gonderim bitti kabul et.
        if (tout > 100)
        {
           tout = 0; eth_rx_len = eth_rd_idx; eth_rd_idx = 0;
            
            
           /* int id = 0; //geleni gostermek icin burayi ac.
             while (eth_rx_len != id)
            {
              gelen_paket += (char)eth_rx_buf[id++];
            }
            //eth_rx_len = 0;
            Serial.println(gelen_paket);
            gelen_paket = "";*/
           
           break;
        }
      }
    } 
}

void parse_response(void)
{
  int scanidx = 0; 
  int pos = 0;
  int  val = 0;
  //String gelen_paket;
      
  //Serial.println("len");
  //Serial.println(eth_rx_len);
  while (scanidx < eth_rx_len)
  { //burada gelen buffer'da tarama yapıp, essitin solundaki string sayıları int sayiya cevirip
    //tabloda uygun degerlerle esleştirip durumunu güncelliyoruz.
    
    if (eth_rx_buf[scanidx] == 0x3D)
    { //"=" isaretini ara. 
      pos = (int)(eth_rx_buf[scanidx + 1] - 48);
      
      //Serial.println(eth_rx_buf[scanidx + 1]);
      
      if (((eth_rx_buf[scanidx - 1]) > 0x2F) && ((eth_rx_buf[scanidx - 1]) < 0x3A))
      {
        val = (int)(eth_rx_buf[scanidx - 1] - 0x30);
        
        if (((eth_rx_buf[scanidx - 2]) > 0x2F) && ((eth_rx_buf[scanidx - 2]) < 0x3A))
        {
          val += (int)((eth_rx_buf[scanidx - 2] - 0x30) * 10);
          
          /*if (((eth_rx_buf[scanidx - 3]) > 0x2F) && ((eth_rx_buf[scanidx - 3]) < 0x3A))
          {
            val += (int)((eth_rx_buf[scanidx - 3] - 0x30) * 100);
            
            if (((eth_rx_buf[scanidx - 4]) > 0x2F) && ((eth_rx_buf[scanidx - 4]) < 0x3A))
            {
              val += (int)((eth_rx_buf[scanidx - 4] - 0x30) * 1000);
            }            
          } */         
        } 
      }

      for (int i = 0; i < MAX_DEGISKEN; i++)
      {
        if (val == val_list[i][0])
        {
          
          val_list[i][1] = pos;
          //break;  //exit fo
          //Serial.println(val_list[i][0]);
        }
      }
      if ((scanidx + 10) <= eth_rx_len) scanidx+=10; else scanidx++;
    } else scanidx++;
    
  } 
}

void loop()
{
  //String gelen_paket;
  
  connect_iocp();
  send_registers();
  
  while(1)
  {
    if (client.connected())
    {
      if (client.available() > 0)
        check_response();
      /*else 
      {
        if (eth_rd_idx) tout++;
        delay(1);   //100milisaniye bir sey gelmezse gonderim bitti kabul et.
         if (tout > 100)
         {
            eth_rx_len = eth_rd_idx; eth_rd_idx = 0; 
            tout = 0;
            break;
         }
        }*/

      if (eth_rx_len) //bir paket gelmisse
      {
        parse_response(); //gelen paketten, val_listte olanlar bulunup yanlarina yeni degerleri yazilacak.
        eth_rx_len = 0; //yeni alimlara hazir olalim.

        
        //gelen paketi terminalde gormek icin burayi ac.
        
         /*int id = 0;
         while (eth_rx_len != id)
        {
          gelen_paket += (char)eth_rx_buf[id++];
        }
        eth_rx_len = 0;
        Serial.println(gelen_paket);
        gelen_paket = "";   *////////

        /*if (val_list[0][1] == 1)
          Serial.println("APU Maint On"); //tabiki buraya led sürme gelecek.
        else*/
          //Serial.println("maplenen");
          
          //maplama
          for (int i = 0; i < MAX_DEGISKEN; i++)
          {
            Serial.print(i); Serial.print(":");
            Serial.print(val_list[i][1]); Serial.print("--");
            digitalWrite(i, val_list[i][1]);  //val_list'in indexi port, n'inci elemanin degeri ise portun degeri
            //delay(5);
          }
          Serial.println(" ");
      }
      
    }
    else
    {
      delay(1000);
      connect_iocp();
      send_registers();
    }

    
    
  }
}
