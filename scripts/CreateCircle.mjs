// @ts-check

const options = await input({
    objID: {
        "type": "int",
        "name": "Object ID"
    },
    objCount: {
        "type": "int",
        "name": "Object count"
    },
    radius: {
        "type": "number",
        "name": "Radius",
        "description": "Circle radius in units (30 units = 1 block)"
    },
});

const pos = editor.getViewCenter();

let angle = 0;
for (let i = 0; i < options.objCount; i += 1) {
    const obj = editor.createObject(options.objID);
    obj.x = pos.x + Math.cos(angle) * options.radius;
    obj.y = pos.y + Math.sin(angle) * options.radius;
    obj.rotation = 360 - angle;
    angle += 360 / options.objCount;
}
