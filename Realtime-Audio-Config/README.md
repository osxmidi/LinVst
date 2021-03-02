Some distros such as Manjaro/Mint don't seem to currently setup audio for realtime 

If they are not set then cpu spiking can occur with plugins.

Installing jackd2 can set up realtime as well.

To set audio realtime priorities manually, edit the below files.

```

sudo edit /etc/security/limits.conf

add

@audio - rtprio 99

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

Also, installing the rtirq-init (rtirq for Manjaro) and irqbalance packages might be useful.
