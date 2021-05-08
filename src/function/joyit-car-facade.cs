using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Threading.Tasks;
using JoyItCar.Function.Intents;
using JoyItCar.Function.Intents.GoogleDialogFlow;
using JoyItCar.Services;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;

namespace JoyItCar.Function
{
    public class joyit_car_facade
    {
        private readonly VehicleService vehicleService;

        public joyit_car_facade(VehicleService vehicleService)
        {
            this.vehicleService = vehicleService;
        }

        [Function("joyit_car_facade")]
        public async Task<HttpResponseData> Run([HttpTrigger(AuthorizationLevel.Function, "post")] HttpRequestData req,
            FunctionContext executionContext)
        {
            ILogger log = executionContext.GetLogger("joyit_car_facade");

            using (var reader = new StreamReader(req.Body))
            {
                var data = await reader.ReadToEndAsync();
                var query = JsonConvert.DeserializeObject<GoogleDialogFlowQuery>(data);

                var dialogResponse = GoogleDialogFlowResponse.FromQuery(query);

                await vehicleService.ConnectToDeviceAsync();

                switch (query.Intent.Name)
                {
                    case "actions.intent.MAIN":
                        break;
                    case "GoForward":
                        await vehicleService.GoForward();
                        break;
                    case "GoBackward":
                        await vehicleService.GoBackward();
                        break;
                    case "TurnLeft":
                        await vehicleService.TurnLeft();
                        break;
                    case "TurnRight":
                        await vehicleService.TurnRight();
                        break;
                    case "Break":
                        await vehicleService.Break();
                        break;
                    default:                
                        var response = req.CreateResponse(HttpStatusCode.NotFound);

                        await response.WriteStringAsync($"Intent {query.Intent.Name} is not handled!");

                        return response;
                }

                dialogResponse.Prompt.FirstSimple = new Simple
                {
                    Speech = "Ok"
                };

                dialogResponse.Prompt.Override = false;

                var httpResponse = req.CreateResponse(HttpStatusCode.OK);
                await httpResponse.WriteAsJsonAsync(dialogResponse);

                return httpResponse;
            }
        }
    }
}
