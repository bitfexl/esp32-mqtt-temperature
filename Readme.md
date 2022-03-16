# Esp32 MQTT Temperature client

**Setup:**

Rename *include/config.h.example* to *config.h* and edit WIFI/MQTT config.



# Mosquitto

```bash
sudo apt install mosquitto mosquitto_clients -y
```



**Config:**

*/etc/mosquitto/mosquitto.conf*

```
# Place your local configuration in /etc/mosquitto/conf.d/
#
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf.example

user mosquitto

pid_file /run/mosquitto/mosquitto.pid

# persistence true
# persistence_location /var/lib/mosquitto/

log_dest file /var/log/mosquitto/mosquitto.log

listener 1883
allow_anonymous true

# $SYS/ Broker Status
sys_interval 10

password_file /etc/mosquitto/password_file
acl_file /etc/mosquitto/acl_file

# include_dir /etc/mosquitto/conf.d
```



*/etc/mosquitto/acl_file*

```
# root user
user root
topic readwrite #

# anonymous (all) users
pattern readwrite anon/#

# user directory
pattern readwrite user/%u/#
```



**Add User:**

```bash
sudo mosquitto_passwd -b /etc/mosquitto/password_file USERNAME PASSWORD
```



**Log:**

```bash
mosquitto_sub -h localhost -v -u root -P root -t \# | ts '[%Y-%m-%d %H:%M:%S]'
```

(*sudo mosquitto_passwd -b /etc/mosquitto/password_file root root*)

Moreutils required:

```bash
sudo apt install moreutils -y
```

