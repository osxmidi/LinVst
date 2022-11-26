Some distros such as Manjaro/Mint/Ubuntu don't seem to currently setup audio for realtime 

If they are not set then cpu spiking can occur with plugins.

Installing jackd2 can set up realtime as well.

For Reaper, also set RT priority to 40 in Audio device settings if using Alsa.

To set audio realtime priorities manually, edit the below files.

```

sudo edit /etc/security/limits.conf

add

@audio - rtprio 95
@audio - memlock unlimited

```

```
run from Terminal

sudo usermod -a -G audio your-user-name

```

```

sudo edit /etc/security/limits.d/audio.conf

add

@audio - rtprio 95
@audio - memlock unlimited
#@audio - nice -19

```

Also, installing the rtirq-init (rtirq for Arch/EndeavourOS/Manjaro) and irqbalance packages might be useful.

rtirq is enabled when threadirqs is added to grub (applies to linux-lowlatency and linux-generic and liquorix and zen kernels)

sudo edit /etc/default/grub 

GRUB_CMDLINE_LINUX="threadirqs"

sudo update-grub or for arch based systems do a system update pacman -Syu

adding "xhci_hcd" to RTIRQ_NAME_LIST in the rtirq.conf file might be needed

sudo edit /etc/rtirq.conf 
or sudo edit /etc/default/rtirq

#RTIRQ_NAME_LIST="snd usb i8042"

RTIRQ_NAME_LIST="xhci_hcd snd usb i8042"

After reboot, check rtirq status
sudo /usr/bin/rtirq status
or sudo /etc/init.d/rtirq status
