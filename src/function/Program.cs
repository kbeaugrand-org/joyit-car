using System.Threading.Tasks;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.Hosting;
using Microsoft.Azure.Functions.Worker.Configuration;
using Microsoft.Extensions.DependencyInjection;
using System;
using Microsoft.Azure.Devices;
using Azure.Core.Serialization;
using System.Text.Json;
using Microsoft.Azure.Functions.Worker;
using JoyItCar.Services;

namespace function
{
    public class Program
    {
        public static void Main()
        {
            var host = new HostBuilder()
                .ConfigureFunctionsWorkerDefaults(ConfigureFunctionsWorker, opts =>
                {
                    opts.Serializer = new JsonObjectSerializer(new JsonSerializerOptions
                    {
                        PropertyNamingPolicy = JsonNamingPolicy.CamelCase
                    });
                })
                .ConfigureServices(ConfigureServices)
                .Build();

            host.Run();
        }

        private static void ConfigureFunctionsWorker(HostBuilderContext context, IFunctionsWorkerApplicationBuilder builder)
        {

        }

        private static void ConfigureServices(HostBuilderContext context, IServiceCollection services)
        {
            var ioTHubConnectionString = context.Configuration.GetValue<string>("IoTHubConnectionString");

            services.AddScoped<VehicleService>();
            
            services.AddSingleton(sp => ServiceClient.CreateFromConnectionString(ioTHubConnectionString));
            services.AddSingleton(sp => RegistryManager.CreateFromConnectionString(ioTHubConnectionString));
        }
    }
}