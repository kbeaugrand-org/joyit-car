using System;
using System.Collections.Generic;
using System.Text;

namespace JoyItCar.Function.Intents.GoogleDialogFlow
{
    public class Scene
    {
        public string Name { get; set; }

        public string SlotFillingStatus { get; set; }

        public Scene Next { get; set; }
    }
}
