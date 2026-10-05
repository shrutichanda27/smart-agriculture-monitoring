# UML Diagram Specifications

Use these specifications to create the final graphical UML diagrams.

## Class Diagram

```text
+------------------+
| SerialReader     |
+------------------+
| - device         |
| - baudRate       |
+------------------+
| +open()          |
| +read()          |
| +close()         |
+--------+---------+
         |
         v
+------------------+
| SensorProcessor  |
+------------------+
| +parse()         |
| +validate()      |
| +process()       |
+--------+---------+
         |
         v
+------------------+       +------------------+
| AlertEngine      |------>| Alert            |
+------------------+       +------------------+
| +evaluate()      |
| +generate()      |
+--------+---------+
         |
         v
+------------------+
| Database         |
+------------------+
| +insertReading() |
| +insertAlert()   |
| +query()         |
+------------------+

+------------------+
| HttpServer       |
+------------------+
| +start()         |
| +handleRequest() |
+--------+---------+
         |
         v
+----------------------+
| DashboardGenerator   |
+----------------------+
| +generateDashboard() |
+----------------------+
```

## Sequence Diagram

```text
Arduino       Linux Driver    SerialReader   Processor   AlertEngine   DB   HTTP/Browser
   |               |              |             |            |          |        |
   |--sensor data->|              |             |            |          |        |
   |               |--TTY data--->|             |            |          |        |
   |               |              |--raw data-->|            |          |        |
   |               |              |             |--reading-->|          |        |
   |               |              |             |            |--alert-->|        |
   |               |              |             |            |--store-->|        |
   |               |              |             |            |          |        |
   |               |              |             |            |          |<--GET--|
   |               |              |             |            |          |        |
   |               |              |             |            |          |--data->|
   |               |              |             |            |          |        |
   |               |              |             |            |          |--HTML->|
```

## State Machine Diagram

```text
          +-------+
          | START |
          +---+---+
              |
              v
       +-------------+
       | INITIALIZE  |
       +------+------+
              |
              v
       +-------------+
       | CHECK DEVICE|
       +------+------+ 
              |
       +------+------+
       |             |
      YES            NO
       |             |
       v             v
+-------------+  +----------------+
| CONNECTED   |  | DEMO/SERVER    |
+------+------+  | ONLY           |
       |         +-------+--------+
       |                 |
       +--------+--------+
                |
                v
       +----------------+
       | READ/GENERATE  |
       | SENSOR DATA    |
       +-------+--------+
               |
               v
       +----------------+
       | VALIDATE DATA  |
       +-------+--------+
               |
          +----+----+
          |         |
        NORMAL    ABNORMAL
          |         |
          v         v
      +-------+ +---------+
      | STORE | |  ALERT  |
      +---+---+ +----+----+
          |          |
          +----+-----+
               |
               v
       +----------------+
       | SERVE DASHBOARD|
       +-------+--------+
               |
               v
          +---------+
          |SHUTDOWN |
          +---------+
```
