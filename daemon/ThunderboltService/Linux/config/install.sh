echo Installing depandencies...
echo Install libdbus-c++
[ -f /usr/lib64/libdbus-c++-1.so ] && echo "libdbus-c++-1.so already installed." || cp libdbus-c++-1.so /usr/lib64/
cp libdbus-c++-1.so /usr/lib64/
[ -f /usr/lib64/libdbus-c++-1.so.0 ] && echo "libdbus-c++-1.so.0 already installed." || ln -s /usr/lib64/libdbus-c++-1.so /usr/lib64/libdbus-c++-1.so.0

echo Installing Thunderbolt daemon binary

chmod +x thunderboltd

[ -d /usr/lib/thunderbolt ] && echo "Thunderbolt daemon already installed. Will be updated to new version" || mkdir /usr/lib/thunderbolt

cp thunderboltd /usr/lib/thunderbolt

echo configure Thunderbolt daemon service
cp thunderbolt.conf /etc/dbus-1/system.d/
cp com.Intel.Thunderbolt.service /usr/share/dbus-1/system-services/.
cp thunderbolt.service /usr/lib/systemd/system/

echo installing udev rules
cp 10-thunderbolt.rules /etc/udev/rules.d
udevadm control --reload-rules

systemctl enable thunderbolt
systemctl daemon-reload

echo Thunderbolt daemon installation done!
