import './Menu.css'
import {useGetPageDescriptors} from "../../contexts/NavigationContext";
import MenuItem from "./MenuItem";

export default function Menu() {

    const descriptors = useGetPageDescriptors();

    return (<div className='menu'>
        {descriptors.map(descriptor => <div><MenuItem pageDescriptor={descriptor}/></div>)}
    </div>)

}