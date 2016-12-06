echo stopping Thunderbolt daemon
systemctl stop thunderbolt
rm /usr/lib64/libdbus-c++-1.so /usr/lib64/libdbus-c++-1.so.0 /usr/lib64/libdbus-c++-1.so.0.0.0 /usr/share/dbus-1/system-services/com.Intel.Thunderbolt.service /etc/dbus-1/system.d/thunderbolt.conf /usr/lib/systemd/system/thunderbolt.service 
rm -r /usr/lib/thunderbolt
echo uninstalling udev rules
rm /etc/udev/rules.d/10-thunderbolt.rules
