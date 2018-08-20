mv wpa_supplicant.conf.tmp /etc/wpa_supplicant/wpa_supplicant.conf
if [ -f utwired.tmp ]
  then
    mv utwired.tmp /etc/init.d/utwired
fi
mv utwireless.tmp /etc/init.d/utwireless
if [ -a /etc/init.d/utwired ]
  then
    chmod +x /etc/init.d/utwired
fi
chmod +x /etc/init.d/utwireless
/etc/init.d/utwireless stop
/etc/init.d/utwireless start
