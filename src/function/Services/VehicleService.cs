using System;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Extensions.Configuration;

namespace JoyItCar.Services
{
    public class VehicleService
    {
        private readonly IConfiguration configuration;

        private readonly RegistryManager registryManager;

        private readonly ServiceClient serviceClient;

        private Device device;

        public VehicleService(IConfiguration configuration, RegistryManager registryManager, ServiceClient serviceClient)
        {
            this.configuration = configuration;
            this.registryManager = registryManager;
            this.serviceClient = serviceClient;
        }

        private void EnsureIsConnected()
        {
            if (device == null)
            {
                throw new InvalidProgramException("Service not initialized. Please call ConnectToDeviceAsync method before!");

            }

            if (device.ConnectionState == DeviceConnectionState.Disconnected)
            {
                throw new InvalidOperationException("Device is not connected!");
            }
        }

        public async Task ConnectToDeviceAsync()
        {
            device = await registryManager.GetDeviceAsync(this.configuration.GetValue<string>("DEVICE_ID"));

            if (device == null)
            {
                throw new InvalidOperationException("Device is not present in the registry!");
            }

            if (device.ConnectionState == DeviceConnectionState.Disconnected)
            {
                throw new InvalidOperationException("Device is not connected!");
            }
        }

        public async Task GoForward()
        {
            await this.SendMethod(nameof(GoForward));
        }

        public async Task GoBackward()
        {
            await this.SendMethod(nameof(GoBackward));
        }

        public async Task Break()
        {
            await this.SendMethod(nameof(Break));
        }

        public async Task TurnLeft()
        {
            await this.SendMethod(nameof(TurnLeft));
        }

        public async Task TurnRight()
        {
            await this.SendMethod(nameof(TurnRight));
        }

        private async Task SendMethod(string methodName)
        {
            this.EnsureIsConnected();
            
            var response = await this.serviceClient.InvokeDeviceMethodAsync(
                deviceId: this.device.Id, 
                new CloudToDeviceMethod(methodName));
        }
    }
}