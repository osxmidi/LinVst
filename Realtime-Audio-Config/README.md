Some distros such as Manjaro/Mint don't seem to currently setup audio for realtime 

If they are not set then cpu spiking can occur with plugins.

To set audio realtime priorities edit the below files.

------

sudo edit /etc/security/limits.conf

add

@audio - rtprio 99

------

sudo edit /etc/group

change

audio:x:29:pulse

to audio:x:29:pulse,<your_username>

------------

sudo edit /etc/security/limits.d/audio.conf

@audio - rtprio 95
@audio - memlock unlimited
#@audio - nice -19
