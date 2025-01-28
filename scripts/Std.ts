
declare interface GameObject {
    readonly id: number;
}
declare interface Editor {
    getSelectedObjects(): GameObject[];
}
declare const editor: Editor;
declare function print(msg: string): void;
