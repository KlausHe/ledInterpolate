
void DemoPosLoopUpdate()
{
  if (positionReached())
  {
    if (demoStepEnable)
    {
      DPRINT("Done");
      timer.disable(Config.drawLoop);
      timer.disable(DemoConfig.posLoop);
      return;
    }
    Position.target = random(Position.min, Position.max);
    saveDierction();
    DPRINT("New random: ");
    DPRINTLN(Position.target);
    return;
  };
  Position.actual += DemoConfig.posSpeed * Position.direction;
}

void saveDierction()
{
  Position.direction = sign(Position.target - Position.actual);
}