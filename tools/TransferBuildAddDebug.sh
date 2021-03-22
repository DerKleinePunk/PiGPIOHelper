#Remember to use ssh-keyscan <SERVER> > ~/.ssh/known_hosts to persist the SFTP server fingerprint to avoid interactive prompting

SFTP_SERVER=$RPI_IP
SFTP_USER=$RPI_USER
SFTP_PWD=$RPI_PASSWORD
currentdir=$(pwd)

if [[ -z "$SFTP_SERVER" ]]; then
   SFTP_SERVER="192.168.2.51"
fi

if [[ -z "$SFTP_USER" ]]; then
   SFTP_USER="pi"
fi

if [[ -z "$SFTP_PWD" ]]; then
   SFTP_PWD="raspberry"
fi

pscp -batch -pw "$SFTP_PWD" $currentdir/../bin/PI/PiGpIoConsole.bin pi@$SFTP_SERVER:/home/pi/devtest/
pscp -batch -pw "$SFTP_PWD" $currentdir/../bin/PI/loggerConsole.conf pi@$SFTP_SERVER:/home/pi/devtest/

echo "Starting GdbServer on Raspberry PI"
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'cd /home/pi/devtest'
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'chmod 771 ./PiGpIoConsole.bin'
sshpass -p $SFTP_PWD ssh $SFTP_USER@$SFTP_SERVER 'gdbserver localhost:9999 ./PiGpIoConsole.bin'
echo "GdbServer on Raspberry PI stopped, ready for next session."
