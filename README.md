# Joyit - Car

Ce projet contient le code source utilisé pour réaliser une voiture connectée avec une Carte Azure Sphere. 

Le code source contient les éléments suivants : 

- [function](./src/function) : 
    Fonction Azure permettant de déclencher des appels de méthode direct sur a voiture connetée sur Azure IoT Hub.
- [JoyitCar](./src/JoyitCar) : 
    Code Azure Sphere HL Core contenant toutes les librairies et code source permettant de manipuler les moteurs de la voiture, se connecter à IoT Hub, recevoir des commandes sur Bluetooth LE ...

# Détails de l'implémentation

Vous pourrez retrouver plus de détails sur comment ce projet a été réalisé sur mon blog : [https://kbeaugrandblog.wordpress.com/2021/05/08/tutoriel-de-la-voiture-connectee-azure-sphere](https://wordpress.com/post/kbeaugrandblog.wordpress.com/622)