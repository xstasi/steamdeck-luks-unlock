
# What is this?

This repository contains an idea as to how to be able to boot an encrypted Debian installation on a Steam Deck.

It attempts to answer the question: how do you unlock an encrypted filesystem without a keyboard?

This project is sort of inspired by pmkap's [effort](https://github.com/pmkap/deckrypt), but rather than using their approach of using the joypad as a joypad and using key combinations as password, I prefer to use a more traditional and more mnemonic alphanumeric approach.

## How does it work?

The idea is to use Steam Deck controller's [lizard mode](https://www.reddit.com/r/SteamController/comments/41329r/eli5_what_is_lizard_mode/). Until Steam is opened, the D-Pad behaves like arrow keys and the right-hand buttons behave like control keys (most importantly, the **A** button behaves like Enter).

This is how the unlocker looks like:

![Unlocker screenshot](/extra/password.png)


Using the D-Pad + A controls you can select the letters that make up your password. Once the letters are selected and "accept" is pressed, the chosen letters are translated to a binary string and passed to LUKS.

This means that your password is limited to one time per letter, but it should not be a problem - hopefully your password wasn't going to be "aaaaaaaaaa" anyway.  In the screenshot above the password was "foobar", so I selected F,O,B,A,R. While this is not perfect security wise, the possible combinations are 2^36 (around 68B) which I believe is good enough.

Another reason why I like this approach is that it gives an old-school game protection vibe.  If you find yourself to be more of a graphical person, the nice square shape allows you to use geometric shapes too instead of alphanumeric passwords. You do you.

## Why Debian?

It is my favourite distribution, so my effort was directed towards it. The good thing is that this project should work on Ubuntu as well without too much hassle - I haven't tried though.

The reason for this project not being "Linux-universal" is not only based on personal taste but also technical limitation: every distribution implements disk encryption differently, and save for the password program itself, everything else must be redone.

Furthermore this is made possible by some Debian specific features of cryptsetup/initrd. In order for this to work on other distros one would have to do a lot of research and testing. I am not really interested in doing it since my distribution has everything I need, but if you do, I'll be happy to hear about it.

# Amazing! How do I do it?

Using this is fairly straightforward if you have some experience with Debian and Linux in general.

Since the device is fairly new at the time of writing, very few people have it and the interest for dual booting Linux+SteamOS seems to be relatively low, there is little information around on how to install another Linux distro in the first place. For this reason I am including a mini-guide on it.

## Installing Debian

I am going to assume that you know how to install Debian normally (including flashing installer on USB dongles etc).

I will also assume that you have a dock for the Steam Deck, or a combination of adapters and/or hubs that allow you to plug a USB dongle and a USB keyboard.

When the system installation will be done you won't need those anymore, but for the installation they are needed.

These are the steps:

* Download iso with firmware from [here](https://cdimage.debian.org/cdimage/unofficial/non-free/cd-including-firmware/current/amd64/iso-cd/) and prepare the installer dongle
* Connect the dongle and keyboard to the deck
* Boot the deck in bootloader mode
  * Power off the deck, power it on in boot mode by pressing **VolDown** + **Power**.
  * Keep pressing **VolDown** until you hear the blip and see the logo
  * This only works while powered off, rebooting doesn't work
* Install Debian normally
  * The screen will be wonky and rotated, but otherwise the installer works
  * Partitioning
    * I personally kept SteamOS and installed Debian alongside it by resizing /home
    * I also tried installing Debian on an external drive and it worked, so I assume if you don't care about SteamOS you can wipe it too but **I HAVE NOT VERIFIED THIS**.
    * I chose to resize /home by -100 GB and partition as follows:
      * No swap partition because I'll use a swap file
      * 4GB ext4 /boot
      * 96GB physical volume for crypto
      * Crypto partition used as ext4 fs, /
  * Choose any passphrase you like, it will be your "backup"
  * Finish the installation
  * Debian will identify SteamOS and add it to GRUB, but be aware that booting SteamOS from GRUB doesn't work. You will have to enter the boot menu every time you want to boot your "other" OS. You can change the default entry by using `efibootmgr` from inside either OS.

After your system is installed you may want to install some graphical interface, and to configure your login manager to log your user in without password. Doing so is dependant on what is your favourite desktop manager, and is beyond the scope of this guide.

## Installing the unlocker

In the future I will distribute premade .deb packages, but for now it's much simpler to build them on your system. Don't worry, it's easy.

I will assume that you have configured sudo, and that you are running these commands as your user.

* Clone this repo on ~user
  * `git clone https://github.com/xstasi/steamdeck-luks-unlock`
* Install dependencies
  * `sudo apt install dpkg-dev gcc debhelper-compat libncurses-dev`
* Build the package
  * `dpkg-buildpackage`
* Install it
  * `sudo dpkg -i ~/steamdeck-luks-unlock_*_amd64.deb`

## Using the unlocker

* Generate a new password for your disk using the unlocker
  * `deck-unlock > ~/mynewpwd`
* Add the new password to your encrypted partition. Change the partition name to suit your setup.
  * `sudo cryptsetup luksAddKey /dev/nvmen1p10 $HOME/mynewpwd`
  * `rm ~/mynewpwd`
* Edit /etc/crypttab, and add the instruction to use this unlocker for your root partition: `keyscript=/usr/bin/deck-unlock`. Example line:
  * `nvme1n1p10_crypt UUID=1234567890abcdef none luks,discard,keyscript=/usr/bin/deck-unlock`
* Regenerate the initramfs
  * `sudo update-initramfs -u`


# My system is broken! 

In the limited testing that I made on my device everything was running smoothly, but it may happen that something breaks for you and you need to restore the system to a state where you have the "old" decryption prompt.

A very easy way to do it is if you have an older kernel (maybe you installed updates before enabling the unlocker?) - if you do just boot the old kernel from grub and you will have the old keyboard based prompt.

If you don't, this is how you get it back:

* Boot into initramfs from grub. Edit the entry (`e` key) and add this at the end of the `linux` line:
  * `break=modules`
* Open the root partition with your backup password. **WARNING**: the last parameter **MUST** match whatever you have on `crypttab`! usually this is `<partition_name>_crypt`, but you can check inside the initramfs.
  * `cryptsetup luksOpen /dev/nvme0n1p10 nvme0n1p10_crypt`
* Mount the root partition (this may require additional steps if you used lvm during installation, adjust mount parameters according to your setup)
  * `mkdir /mnt`
  * `mount -t ext4 /dev/mapper/root /mnt`
  * `mount -t ext4 /dev/nvme0n1p9 /mnt/boot`
  * `for i in sys proc dev run ; do mount /$i -o bind /mnt/$i; done`
  * `chroot /mnt`
* From inside the chroot, regenerate the initramfs
  * `update-initramfs -u`
* Leave the chroot and unmount
  * `for i in sys proc dev run ; do umount /mnt/$i; done`
  * `umount /mnt/boot`
  * `umount /mnt/`
* Power off and back on, and you should boot into the usual way
