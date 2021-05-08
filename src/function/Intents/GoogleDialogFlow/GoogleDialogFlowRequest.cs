using System;
using System.Collections.Generic;
using System.Text;

namespace JoyItCar.Function.Intents.GoogleDialogFlow
{
    public class Handler
    {
        public string Name { get; set; }
    }

    public class GoogleDialogFlowQuery
    {
        public Handler Handler { get; set; }

        public Intent Intent { get; set; }

        public Scene Scene { get; set; }

        public Session Session { get; set; }

        public Device Device { get; set; }
    }
}
