using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Text;

namespace JoyItCar.Function.Intents.GoogleDialogFlow
{
    public class GoogleDialogFlowResponse
    {
        private GoogleDialogFlowQuery Query { get; }

        public Session Session => this.Query.Session;

        public Scene Scene => this.Query.Scene;
        
        public Prompt Prompt { get; set; } = new Prompt();

        private GoogleDialogFlowResponse(GoogleDialogFlowQuery request)
        {
            this.Query = request;
        }

        public static GoogleDialogFlowResponse FromQuery(GoogleDialogFlowQuery request)
        {
            return new GoogleDialogFlowResponse(request);
        }
    }
}
