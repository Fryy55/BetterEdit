
declare interface Metadata {
    name: string,
    by: string,
}

declare interface EditorEventListeners {
    "select": (objs: GameObject[]) => void,
}

/**
 * Represents an object in the editor
 */
declare class GameObject {
    /**
     * The ID of the object. See [Colon's level](https://gdbrowser.com/99784974) 
     * for a list of all object IDs. This property is not modifiable; if you 
     * want to convert an object to a different type, use {@link Editor.createObject} 
     * and transfer the properties you want over to the new one
     */
    readonly id: number;
    /**
     * The X-coordinate of the object. Modifying this property will move the 
     * object. Note that for performance reasons, you should use 
     * {@link Editor.moveObjectsBy} instead if you are moving large amounts of 
     * objects
     */
    x: number;
    /**
     * The X-coordinate of the object. Modifying this property will move the 
     * object. Note that for performance reasons, you should use 
     * {@link Editor.moveObjectsBy} instead if you are moving large amounts of 
     * objects
     */
    y: number;
    /**
     * Whether this object is selected or not. Modifying this property will 
     * select/deselect the object
     */
    selected: boolean;
}
/**
 * Interface for interacting with the level editor
 */
declare class Editor {
    /**
     * Get the currently selected objects
     */
    getSelectedObjects(): GameObject[];
    /**
     * Move the specified objects by the specified amount of units relative to 
     * their current position
     * @param objs Objects to move
     * @param amount Amount to move relative to the objects' current position, 
     * in (X, Y) coordinates
     */
    moveObjectsBy(objs: GameObject[], amount: [number, number]): void;
    /**
     * Create a new object in the editor. The object is placed at (0, 0) (the bottom left starting corner)
     * @param id The ID of the object to create. See [Colon's level](https://gdbrowser.com/99784974) for a list of all object IDs
     */
    createObject(id: number): GameObject;

    addEventListener<K extends keyof EditorEventListeners>(event: K, onEvent: EditorEventListeners[K]): void;
}
/**
 * Interface for interacting with the level editor, such as getting selected objects or creating new ones
 */
declare const editor: Editor;
/**
 * Output messages to the script run window
 * @param msgs Message(s) to output
 */
declare function print(...msgs: any[]): void;
